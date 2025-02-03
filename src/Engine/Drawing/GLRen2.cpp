#include "GLRen2.h"

#include <utility>
#include <cstring>

#include "Helpers/VectorHelper.h"
#include "Helpers/ProfileHelper.h"

#include "Program.h"
#include "BindingManager.h"
#include "CubeMap.h"

// Workaround to access profiler
#include "Systems/Execution/Engine.h"
#include <memory>

extern std::unique_ptr<Engine::IEngine> g_Engine;

namespace Drawing
{
	GLuint DrawCallRenderer::MaterialBufBinding = 0;
	GLuint DrawCallRenderer::PerObjectBufBinding = 0;
	GLuint DrawCallRenderer::LightBufBinding = 0;

	struct ProgramDrawCalls
	{
		std::shared_ptr<Program> Program;
		std::vector<DrawCallv2> DrawCalls;
	};

	std::vector<floaty4> getFrustumCornersWorldSpace(const Matrixy4x4& proj, const Matrixy4x4& view)
	{
		const auto inv = Matrixy4x4::InvertedOrIdentity(Matrixy4x4::Multiply(proj, view));

		std::vector<floaty4> frustumCorners;
		for (unsigned int x = 0; x < 2; ++x)
		{
			for (unsigned int y = 0; y < 2; ++y)
			{
				for (unsigned int z = 0; z < 2; ++z)
				{
					const floaty4 pt =
						inv.Transform(floaty4(
							2.0f * x - 1.0f,
							2.0f * y - 1.0f,
							2.0f * z - 1.0f,
							1.0f));
					frustumCorners.push_back(pt / pt.w);
				}
			}
		}

		return frustumCorners;
	}

	floaty3 getFrustumCenterWorldSpace(const Matrixy4x4& proj, const Matrixy4x4& view)
	{
		const auto inv = Matrixy4x4::InvertedOrIdentity(Matrixy4x4::Multiply(proj, view));

		auto p1 = inv.Transform(floaty4{ 0.f, 0.f, -1.f, 1.f });
		auto p2 = inv.Transform(floaty4{ 0.f, 0.f, 1.f, 1.f });
		p1 /= p1.w;
		p2 /= p2.w;

		return ((p1 + p2) / 2.f).xyz();
	}

	GLProgram DrawCallRenderer::CreateShadowProgram(const std::string& vertexShader, const std::string& fragmentShader)
	{
		CHECK_GL_ERR("Before shadow program creation");

		GLProgram prog = GLProgram{ std::vector<GLShaderPair>{ { vertexShader, GL_VERTEX_SHADER }, { fragmentShader, GL_FRAGMENT_SHADER } } };

		auto blockIndex = glGetUniformBlockIndex(prog.Get(), "PerObject");
		if (blockIndex != GL_INVALID_INDEX)
			glUniformBlockBinding(prog.Get(), blockIndex, GetPerObjectBufBinding());
		else
			DERROR("Could not find 'PerObject' binding for the shadow program");

		CHECK_GL_ERR("After shadow program creation");
		return prog;
	}

	GLFrameBuffer DrawCallRenderer::CreateShadowFBO()
	{
		GLuint out = 0;
		glGenFramebuffers(1, &out);

		glBindFramebuffer(GL_FRAMEBUFFER, out);
		//glNamedFramebufferDrawBuffer(out, GL_NONE);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return out;
	}

	void DrawCallRenderer::DrawPointShadows(const Light& light, GLuint mapSize)
	{
		auto shadowIndex = light.ShadowIndex;
		auto& shadowTex = *_shadowTextures[shadowIndex - 1];
		if (!_shadowTextures[shadowIndex - 1] || _shadowTextures[shadowIndex - 1]->GetTarget() != GL_TEXTURE_CUBE_MAP)
		{
			_shadowTextures[shadowIndex - 1] = std::make_unique<Drawing::GLCubeMap>(mapSize, mapSize, GL_DEPTH_COMPONENT24);
			glTextureParameteri(shadowTex.Get(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(shadowTex.Get(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			GLfloat borderColor[3] = { 0.f, 0.f, 0.f };
			glTextureParameterfv(shadowTex.Get(), GL_TEXTURE_BORDER_COLOR, borderColor);
		}

		struct cubemapSide
		{
			GLenum attachment;
			floaty3 target;
			floaty3 up;
		};
		static const std::array<cubemapSide, 6> mapSides = { 
			cubemapSide{GL_TEXTURE_CUBE_MAP_POSITIVE_X, { +1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f } }, 
			cubemapSide{GL_TEXTURE_CUBE_MAP_NEGATIVE_X, { -1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f } },
			cubemapSide{GL_TEXTURE_CUBE_MAP_POSITIVE_Y, { 0.f, +1.f, 0.f }, { 1.f, 0.f, 0.f } },
			cubemapSide{GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, { 0.f, -1.f, 0.f }, { 1.f, 0.f, 0.f } },
			cubemapSide{GL_TEXTURE_CUBE_MAP_POSITIVE_Z, { 0.f, 0.f, +1.f }, { 0.f, 1.f, 0.f } },
			cubemapSide{GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, { 0.f, 0.f, -1.f }, { 0.f, 1.f, 0.f } },
		};
		for (int i = 0; i < 6; ++i)
		{
			auto& side = mapSides[i];
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, side.attachment, shadowTex.Get(), 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			glViewport(0, 0, mapSize, mapSize);
			glClear(GL_DEPTH_BUFFER_BIT);

			floaty3 lightPos = light.PositionWS.xyz();
			Matrixy4x4 lightView = Matrixy4x4::LookAt(lightPos, lightPos + side.target, side.up);
			Matrixy4x4 lightProjection = Matrixy4x4::PerspectiveFovD(90.f, 1.f, 0.1f, light.Range);

			DrawShadowGeometry(Matrixy4x4::Multiply(lightProjection, lightView));
		}
	}

	void DrawCallRenderer::DrawSpotlightShadows(const Light& light, GLuint mapSize)
	{
		auto shadowIndex = light.ShadowIndex;
		auto& shadowMap = _shadowTextures[shadowIndex - 1];
		if (!shadowMap || shadowMap->GetTarget() != GL_TEXTURE_2D)
		{
			shadowMap = std::make_unique<GLImage>(Create2DGLTexture(GL_DEPTH_COMPONENT24, mapSize, mapSize));
			glTextureParameteri(shadowMap->Get(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(shadowMap->Get(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			GLfloat borderColor[3] = { 0.f, 0.f, 0.f };
			glTextureParameterfv(shadowMap->Get(), GL_TEXTURE_BORDER_COLOR, borderColor);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap->Get(), 0);

		glViewport(0, 0, mapSize, mapSize);
		glClear(GL_DEPTH_BUFFER_BIT);

		// There's a small possibility the light's direction will be parallel to this completely random vector
		// If it is it in theory produces the zero vector which will ruin the view matrix
		floaty3 lightPos = light.PositionWS.xyz();
		floaty3 lightDir = light.DirectionWS.xyz();
		floaty3 up = lightDir.cross(floaty3{ 0.15f, 0.15f, 0.051f });
		if (up.mag2() == 0.f)
			up = lightDir.cross(floaty3{ 0.f, 1.f, 0.f });
		up.normalize();

		Matrixy4x4 lightView = Matrixy4x4::LookAt(lightPos, lightPos + lightDir, up);
		Matrixy4x4 lightProjection = Matrixy4x4::PerspectiveFovR(light.SpotlightAngle * 2.f, 1.f, 0.25f, light.Range);
		auto lightMatrix = Matrixy4x4::Multiply(lightProjection, lightView);
		_shadowMatrices[shadowIndex - 1] = lightMatrix;

		DrawShadowGeometry(lightMatrix);
	}

	void DrawCallRenderer::DrawSunShadows(const Light& light, Matrixy4x4 view, Matrixy4x4 proj, GLuint mapSize)
	{
		std::array<GLuint, CascadeCount> cascadeMapSizes = { mapSize, mapSize, mapSize / 2 };


		for (size_t i = 0; i < CascadeCount; ++i)
		{
			auto size = cascadeMapSizes[i];
			auto& shadowMap = _shadowCascadeTextures[i];
			if (shadowMap.GetTarget() != GL_TEXTURE_2D)
			{
				shadowMap = GLImage(Create2DGLTexture(GL_DEPTH_COMPONENT24, size, size));
				glTextureParameteri(shadowMap.Get(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTextureParameteri(shadowMap.Get(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				GLfloat borderColor[3] = { 0.f, 0.f, 0.f };
				glTextureParameterfv(shadowMap.Get(), GL_TEXTURE_BORDER_COLOR, borderColor);
			}


			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap.Get(), 0);
			glViewport(0, 0, size, size);
			glClear(GL_DEPTH_BUFFER_BIT);

			auto myNear = _shadowCascadeDistances[i];
			auto myFar = _shadowCascadeDistances[i + 1];

			
			auto myProj = proj;
			myProj.m[2][2] = (myFar + myNear) / (myNear - myFar);
			myProj.m[3][2] = (2.f * myFar * myNear) / (myNear - myFar);

			auto corners = getFrustumCornersWorldSpace(myProj, view);
			auto center = getFrustumCenterWorldSpace(myProj, view);

			//floaty3 center{ 0.f, 0.f, 0.f };
			//for (const auto& v : corners)
			//{
			//	center += v.xyz();
			//}

			//center /= corners.size();

			floaty3 up;
			if (light.DirectionWS.y == 1.f || light.DirectionWS.y == -1.f)
				up = floaty3{ 1.f, 0.f, 0.f };
			else 
				up = floaty3{ 0.f, 1.f, 0.f };

			const auto lightView = Matrixy4x4::LookAt(center - light.DirectionWS.xyz(), center, up);

			float minX = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::min();
			float minY = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::min();
			float minZ = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::min();

			for (auto& corner : corners)
			{
				auto realCorner = lightView.Transform(corner);

				minX = std::min(minX, realCorner.x);
				maxX = std::max(maxX, realCorner.x);
				minY = std::min(minY, realCorner.y);
				maxY = std::max(maxY, realCorner.y);
				minZ = std::min(minZ, realCorner.z);
				maxZ = std::max(maxZ, realCorner.z);
			}

			constexpr float zMult = 5.0f;
			if (minZ < 0.f)
				minZ *= zMult;
			else
				minZ /= zMult;
			if (maxZ < 0.f)
				maxZ /= zMult;
			else
				maxZ *= zMult;


			Matrixy4x4 cascadeProj = Matrixy4x4::OrthoProject(minX, maxX, minY, maxY, minZ, maxZ);

			Matrixy4x4 cascadeViewProj = Matrixy4x4::MultiplyE(lightView, cascadeProj);

			_shadowCascadeMatrices[i] = cascadeViewProj;

			DrawShadowGeometry(cascadeViewProj);
		}
	}

	void DrawCallRenderer::DrawShadows(Matrixy4x4 view, Matrixy4x4 proj)
	{
		GLint viewport[4] = { 0, 0, 0, 0 };
		glGetIntegerv(GL_VIEWPORT, viewport);
		glBindFramebuffer(GL_FRAMEBUFFER, _shadowFBO.Get());
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		GLuint nonDirectionShadowIndex = 1;
		GLuint directionShadowsIndex = 1;

		for (size_t i = 0; i < _lights.size(); ++i)
		{
			auto& light = _lights[i];
			if (!light.Enabled)
				continue;

			switch (light.Type)
			{
			default:
			case LIGHT_POINT:
				if (nonDirectionShadowIndex > MAX_SHADOW_LIGHT_COUNT)
				{
					continue;
				}
				light.ShadowIndex = nonDirectionShadowIndex++;
				DrawPointShadows(light, ShadowMapSize);
				break;
			case LIGHT_SPOT:
				if (nonDirectionShadowIndex > MAX_SHADOW_LIGHT_COUNT)
				{
					continue;
				}
				light.ShadowIndex = nonDirectionShadowIndex++;
				DrawSpotlightShadows(light, ShadowMapSize);
				break;
			case LIGHT_DIRECTION:
				if (directionShadowsIndex > 1)
				{
					//DWARNING("Can not render shadows for more than one directional light");
					continue;
				}
				light.ShadowIndex = directionShadowsIndex++;
				DrawSunShadows(light, view, proj, ShadowMapSize);
				break;
			}
		}

		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}

	void DrawCallRenderer::DrawShadowGeometry(Matrixy4x4 lightViewProj)
	{
		GLuint lastVertexBuffer = 0;
		DefaultPerObjectStruct perObject;
		perObject.Projection = Matrixy4x4::Identity();
		perObject.View = Matrixy4x4::Identity();
		perObject.World = Matrixy4x4::Identity();
		perObject.WorldView = Matrixy4x4::Identity();
		perObject.WorldViewProj = Matrixy4x4::Identity();

		glUseProgram(_shadowProgram.Get());

		for (auto& program_calls_pair : m_DrawCallGroups)
		{
			auto& program = program_calls_pair.first.GetProgram();
			if (!program)
			{
				DWARNING("Invalid program found by name of '" + program->GetName() + "'");
				continue;
			}

			if (!program->GetShadowSupport())
				continue;

			PROFILE_EVENT_WITH(p, g_Engine->Resources.Profile, "Program DrawCalls", true);
			for (auto& drawcall_tmp : program_calls_pair.second)
			{
				auto& drawcall = drawcall_tmp.get();

				if (!drawcall.geometry)
					continue;

				auto& storage = drawcall.geometry->GetStorage();
				if (!storage.Buffer)
				{
					DWARNING("DrawCall '" + drawcall.debugString + "' has no geometry!");
					continue;
				}

				storage.Buffer->UpdateIfDirty();

				MeshOffsetData offsetData;
				if (!storage.Buffer->TryGetMeshOffset(storage.ID, &offsetData))
					continue;

				if (offsetData.IndicesCount < 1)
					continue;

				auto thisBuffer = drawcall.geometry->GetStorage().Buffer->GetVBO().Get();
				if (thisBuffer != lastVertexBuffer)
				{
					program->BindTo(*drawcall.geometry->GetStorage().Buffer);
				}
				lastVertexBuffer = thisBuffer;

				auto& world = *drawcall.matrix;

				perObject.WorldViewProj = Matrixy4x4::MultiplyE(world, lightViewProj);
				UpdateBuffer(_perObjectBuffer, &perObject, sizeof(DefaultPerObjectStruct), m_bufferUpdateMode);

				program->BindVAO();

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawcall.geometry->GetStorage().Buffer->GetIBO().Get());

				glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)offsetData.IndicesCount, GL_UNSIGNED_INT, (GLvoid*)(offsetData.IndexStart * sizeof(GLuint)), (GLint)offsetData.IndexOffset);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				glBindVertexArray(0);

			}

		}
	}

	void DrawCallRenderer::Draw(Matrixy4x4 View, Matrixy4x4 Proj, Voxel::CameraFrustum frustum)
	{
		(void)frustum; // Frustum culling not done

		PROFILE_PUSH_WITH(g_Engine->Resources.Profile, "Renv2");

		GLuint lastVertexBuffer = 0;

		UpdateLights(View);

		CHECK_GL_ERR("Pre-Enabling Depth Test");
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		CHECK_GL_ERR("Enabling Depth Test");

		// Temporarily we will not sort or cull calls
		// auto culled_calls = Cull(calls); Cull here maybe?

		PROFILE_PUSH_WITH(g_Engine->Resources.Profile, "Organising calls");
		if (_drawCallsDirty)
			UpdateDrawCalls();
		PROFILE_POP_WITH(g_Engine->Resources.Profile);

		DrawShadows(View, Proj);

		PROFILE_PUSH_WITH(g_Engine->Resources.Profile, "Executing Draw Calls");
		for (auto& program_calls_pair : m_DrawCallGroups)
		{
			auto& program = program_calls_pair.first.GetProgram();
			if (!program)
			{
				DWARNING("Invalid program found by name of '" + program->GetName() + "'");
				continue;
			}

			program->SetActive();

			PROFILE_EVENT_WITH(p, g_Engine->Resources.Profile, "Program DrawCalls", true);
			for (auto& drawcall_tmp : program_calls_pair.second)
			{

				auto& drawcall = drawcall_tmp.get();

				if (!drawcall.geometry)
					continue;

				auto& storage = drawcall.geometry->GetStorage();
				if (!storage.Buffer)
				{
					DWARNING("DrawCall '" + drawcall.debugString + "' has no geometry!");
					continue;
				}

				storage.Buffer->UpdateIfDirty();

				MeshOffsetData offsetData;
				if (!storage.Buffer->TryGetMeshOffset(storage.ID, &offsetData))
					continue;

				if (offsetData.IndicesCount < 1)
					continue;

				auto thisBuffer = drawcall.geometry->GetStorage().Buffer->GetVBO().Get();
				if (thisBuffer != lastVertexBuffer)
				{
					program->BindTo(*drawcall.geometry->GetStorage().Buffer);
				}
				lastVertexBuffer = thisBuffer;

				auto& world = *drawcall.matrix;

				UpdatePerObject(world, View, Proj);
				UpdateMaterial(*program, *drawcall.material);
				UpdateTextures(*program, *drawcall.material);
				UpdateShadowMaps(*program);
					
				program->BindVAO();

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawcall.geometry->GetStorage().Buffer->GetIBO().Get());

				glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)offsetData.IndicesCount, GL_UNSIGNED_INT, (GLvoid*)(offsetData.IndexStart * sizeof(GLuint)), (GLint)offsetData.IndexOffset);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				glBindVertexArray(0);

			}

		}

		PROFILE_POP_WITH(g_Engine->Resources.Profile);
		glDisable(GL_CULL_FACE);
		PROFILE_POP_WITH(g_Engine->Resources.Profile);
	}

	Light* DrawCallRenderer::GetLight(size_t index)
	{
		return &_lights[index];
	}

	const Light* DrawCallRenderer::GetLight(size_t index) const
	{
		return &_lights[index];
	}

	void DrawCallRenderer::SetLight(size_t index, Light val)
	{
		_lights[index] = val;
	}

	void DrawCallRenderer::UpdateDrawCalls()
	{
		// Perhaps try clear just unused programs instead of the whole thing?
		m_DrawCallGroups.clear();

		for (auto& call_pair : _drawCalls)
		{
			auto& drawCall = call_pair.second;
			if (!drawCall.material || !drawCall.material->GetProgram().IsValid())
				continue;
			const auto& drawCallProgName = drawCall.material->GetProgram().GetProgramName();
			auto& vec = m_DrawCallGroups[drawCallProgName];

			vec.emplace_back(std::cref(drawCall));
		}

		_drawCallsDirty = false;
	}

	void DrawCallRenderer::UpdateLights(Matrixy4x4 view)
	{
		for (int i = 0; i < _lights.size(); ++i)
		{
			auto& l = _lights[i];
			if (l.Enabled)
			{
				l.DirectionVS = view.TransformNormal(l.DirectionWS);
				l.PositionVS = view.Transform(l.PositionWS);
			}
		}

		glNamedBufferSubData(_lightBuffer.Get(), 0, sizeof(Light) * LightCount, _lights.data());
		CHECK_GL_ERR("After Updating Lights");
	}

	void DrawCallRenderer::UpdatePerObject(Matrixy4x4 world, Matrixy4x4 view, Matrixy4x4 proj)
	{
		CHECK_GL_ERR("Pre Updating Per Object");
		PROFILE_PUSH_WITH(g_Engine->Resources.Profile, "Per Object");
		DefaultPerObjectStruct data;
		data.World = world;
		data.View = view;
		data.Projection = proj;
		data.WorldView = Matrixy4x4::Multiply(view, world);
		data.WorldViewProj = Matrixy4x4::Multiply(proj, data.WorldView);

		UpdateBuffer(_perObjectBuffer, &data, sizeof(DefaultPerObjectStruct), m_bufferUpdateMode);
		
		PROFILE_POP_WITH(g_Engine->Resources.Profile);
		CHECK_GL_ERR("Post Updating Per Object");
	}

	void DrawCallRenderer::UpdateMaterial(Program& prog, Material& mat)
	{
		CHECK_GL_ERR("Pre Updating Material");
		PROFILE_PUSH_WITH(g_Engine->Resources.Profile, "Material");
		prog.SetMaterial(mat, m_bufferUpdateMode);
		PROFILE_POP_WITH(g_Engine->Resources.Profile);
		CHECK_GL_ERR("Post Updating Material");
	}

	void DrawCallRenderer::UpdateTextures(Program& prog, const Material& mat)
	{
		CHECK_GL_ERR("Before Updating Textures");
		PROFILE_PUSH_WITH(g_Engine->Resources.Profile, "Textures");
		auto& mappings = prog.GetTexMappings();

		for (int i = 0; i < mappings.size(); ++i)
		{
			auto& mapping = mappings[i];
			auto it = mat.GetTextures().find(mapping.MatName);
			if (it != mat.GetTextures().end())
			{
				const auto& tex = it->second.GetTexture();
				if (!tex)
					continue;
				tex->LoadGL();
				glBindTextureUnit(mapping.Binding, tex->Get());
			}
		}
		PROFILE_POP_WITH(g_Engine->Resources.Profile);
		CHECK_GL_ERR("Post Updating Textures");
	}

	void DrawCallRenderer::UpdateShadowMaps(Program& prog)
	{
		CHECK_GL_ERR("Before updating shadow maps");

		auto map2dBinding = prog.GetShadowMap2DBinding();
		auto mapCubeBinding = prog.GetShadowMapCubemapBinding();
		auto mapCascadeBinding = prog.GetShadowCascadeBinding();
		if (map2dBinding == -1 || mapCubeBinding == -1 || mapCascadeBinding == -1)
			return;

		for (int i = 0; i < ShadowLightCount; ++i)
		{
			auto& shadowMap = _shadowTextures[i];
			if (!shadowMap)
				continue;
			if (shadowMap->GetTarget() == GL_TEXTURE_CUBE_MAP)
			{
				glBindTextureUnit(map2dBinding + i, 0);
				glBindTextureUnit(mapCubeBinding + i, shadowMap->Get());
			}
			else if (shadowMap->GetTarget() == GL_TEXTURE_2D)
			{
				glBindTextureUnit(map2dBinding + i, shadowMap->Get());
				glBindTextureUnit(mapCubeBinding + i, 0);
			}
		}
		for (int i = 0; i < 3; ++i)
		{
			auto& shadowMap = _shadowCascadeTextures[i];
			glBindTextureUnit(mapCascadeBinding + i, shadowMap.Get());
		}


		CHECK_GL_ERR("After updating shadow maps");

		std::vector<Matrixy4x4> matrices{ _shadowMatrices.begin(), _shadowMatrices.end() };
		ShadowCascadeBuffer buffer;
		std::memcpy(&buffer.ProjectionViewMatrices, _shadowCascadeMatrices.data(), sizeof(Matrixy4x4) * 3);
		std::memcpy(&buffer.Distances, &_shadowCascadeDistances[1], sizeof(float) * 3);
		prog.SetShadowMatrices(matrices, buffer, m_bufferUpdateMode);

		CHECK_GL_ERR("After Updating Shadow Matrices");
	}

	GLuint DrawCallRenderer::InitPerObjectBuffer()
	{
		CHECK_GL_ERR("Pre Bind/init per object buffer");
		GLuint out = 0;

		glGenBuffers(1, &out);
		
		if (!PerObjectBufBinding)
			PerObjectBufBinding = BindingManager::GetNext();

		glBindBufferBase(GL_UNIFORM_BUFFER, PerObjectBufBinding, out);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(DefaultPerObjectStruct), nullptr, GL_STREAM_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		CHECK_GL_ERR("Post Bind and init PerObject Buffer");

		return out;
	}

	GLuint DrawCallRenderer::InitLightBuffer()
	{
		if (!LightBufBinding)
			LightBufBinding = BindingManager::GetNext();

		std::memset(_lights.data(), 0, sizeof(_lights));

		GLuint out;
		glGenBuffers(1, &out);

		glBindBufferBase(GL_UNIFORM_BUFFER, LightBufBinding, out);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * LightCount, _lights.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return out;
	}

	DrawCallRenderer::DrawCallRenderer(CommonResources* resources)
		: _drawCalls()
		, _lights()
		, _perObjectBuffer(InitPerObjectBuffer())
		, _lightBuffer(InitLightBuffer())
		, _shadowProgram(CreateShadowProgram("Programs/shadow_voxel_vertex.glvs", "Programs/shadow_voxel_fragment.glfs"))
		, _shadowFBO(CreateShadowFBO())
	{
		(void)resources; // Should it even take it?
		if (!MaterialBufBinding)
			MaterialBufBinding = BindingManager::GetNext();
		if (!PerObjectBufBinding)
			PerObjectBufBinding = BindingManager::GetNext();
		if (!LightBufBinding)
			LightBufBinding = BindingManager::GetNext();
		
#if defined(GL_ARB_seamless_cube_map)
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif
	}

	DrawCallReference DrawCallRenderer::SubmitDrawCall(DrawCallv2 drawCall)
	{
		auto key = _nextKey;
		_nextKey++;

		_drawCalls[key] = std::move(drawCall);

		_drawCallsDirty = true;
		return DrawCallReference{ key, this };
	}

	const DrawCallv2* DrawCallRenderer::GetDrawCall(size_t key) const
	{
		auto it = _drawCalls.find(key);
		if (it != _drawCalls.end())
			return &(it->second);
		return nullptr;
	}

	const DrawCallv2* DrawCallRenderer::GetDrawCall(const DrawCallReference& reference) const
	{
		return GetDrawCall(reference.GetKey());
	}

	bool DrawCallRenderer::SetDrawCall(size_t key, DrawCallv2 call)
	{
		auto it = _drawCalls.find(key);
		auto is_new = it == _drawCalls.end();

		if (is_new)
			_drawCallsDirty = true;
		else
			_drawCallsDirty = it->second.material->GetProgram() != call.material->GetProgram();

		_drawCalls[key] = std::move(call);
		return !is_new;
	}

	bool DrawCallRenderer::SetDrawCall(const DrawCallReference& reference, DrawCallv2 call)
	{
		return SetDrawCall(reference.GetKey(), std::move(call));
	}

	GLuint DrawCallRenderer::GetLightBufBinding()
	{
		if (!LightBufBinding)
			LightBufBinding = BindingManager::GetNext();

		return LightBufBinding;
	}

	GLuint DrawCallRenderer::GetMaterialBufBinding()
	{
		if (!MaterialBufBinding)
			MaterialBufBinding = BindingManager::GetNext();

		return MaterialBufBinding;
	}

	GLuint DrawCallRenderer::GetPerObjectBufBinding()
	{
		if (!PerObjectBufBinding)
			PerObjectBufBinding = BindingManager::GetNext();

		return PerObjectBufBinding;
	}



	bool DrawCallRenderer::RemoveDrawCall(size_t key)
	{
		auto it = _drawCalls.find(key);
		if (it == _drawCalls.end())
			return false;

		_drawCalls.erase(it);
		_drawCallsDirty = true; // Possibly don't recalc draw calls but just remove?
		return true;
	}

	bool DrawCallRenderer::RemoveDrawCall(const DrawCallReference& reference)
	{
		return RemoveDrawCall(reference.GetKey());
	}
}
