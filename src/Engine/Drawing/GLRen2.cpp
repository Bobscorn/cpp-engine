#include "GLRen2.h"

#include <utility>
#include <cstring>

#include "Helpers/VectorHelper.h"
#include "Helpers/ProfileHelper.h"

#include "Program.h"
#include "BindingManager.h"

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

			auto p = PROFILE_EVENT_WITH(g_Engine->Resources.Profile, "Program DrawCalls", true);
			for (auto& drawcall_tmp : program_calls_pair.second)
			{

				auto& drawcall = drawcall_tmp.get();

				if (!drawcall.Geometry)
					continue;

				auto& storage = drawcall.Geometry->GetStorage();
				if (!storage.Buffer)
				{
					DWARNING("DrawCall '" + drawcall.DebugString + "' has no geometry!");
					continue;
				}

				storage.Buffer->UpdateIfDirty();

				MeshOffsetData offsetData;
				if (!storage.Buffer->TryGetMeshOffset(storage.ID, &offsetData))
					continue;

				if (offsetData.IndicesCount < 1)
					continue;

				auto thisBuffer = drawcall.Geometry->GetStorage().Buffer->GetVBO().Get();
				if (thisBuffer != lastVertexBuffer)
				{
					program->BindTo(*drawcall.Geometry->GetStorage().Buffer);
				}
				lastVertexBuffer = thisBuffer;

				auto& world = *drawcall.Matrix;

				UpdatePerObject(world, View, Proj);
				UpdateMaterial(*program, *drawcall.Material);
				UpdateTextures(*program, *drawcall.Material);
					
				program->BindVAO();

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawcall.Geometry->GetStorage().Buffer->GetIBO().Get());

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
			if (!drawCall.Material || !drawCall.Material->GetProgram().IsValid())
				continue;
			const auto& drawCallProgName = drawCall.Material->GetProgram().GetProgramName();
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

		glNamedBufferSubData(_perObjectBuffer.Get(), 0, sizeof(DefaultPerObjectStruct), (GLvoid*)&data);
		PROFILE_POP_WITH(g_Engine->Resources.Profile);
		CHECK_GL_ERR("Post Updating Per Object");
	}

	void DrawCallRenderer::UpdateMaterial(Program& prog, Material& mat)
	{
		CHECK_GL_ERR("Pre Updating Material");
		PROFILE_PUSH_WITH(g_Engine->Resources.Profile, "Material");
		prog.SetMaterial(mat);
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
			_drawCallsDirty = it->second.Material->GetProgram() != call.Material->GetProgram();

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
