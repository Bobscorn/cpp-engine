#include "GLRen2.h"

#include <utility>

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

		PROFILE_PUSH_WITH(g_Engine->Resources.Profile, "Renv2");

		GLuint lastVertexBuffer = 0;

		UpdateLights(View);

		CHECK_GL_ERR("Pre-Enabling Depth Test");
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		CHECK_GL_ERR("Enabling Depth Test");

		// Temporarily we will not sort or cull calls
		// auto culled_calls = Cull(calls); Cull here maybe?

		// Also temporarily we will do a super slow and iterate the drawcalls every frame and collect them into a vector
		std::vector<ProgramDrawCalls> calls;

		PROFILE_PUSH_WITH(g_Engine->Resources.Profile, "Organising calls");
		// Part of temporary-ness, use unordered map to gather all the calls into per-program vectors
		std::unordered_map<std::string, std::vector<DrawCallv2>> programCallMap{};

		for (auto& pair : _drawCalls)
		{
			auto& call = pair.second;
			if (!call.Material || !call.Geometry || !call.Enabled || !call.Geometry->operator bool() || call.Geometry->GetMesh()->VertexData.NumVertices() <= 0)
				continue;
			auto& progName = call.Material->Program.GetProgramName();

			programCallMap[progName].push_back(call);
		}

		// Decompose map into vector
		for (auto& pair : programCallMap)
		{
			auto& progName = pair.first;
			auto& callsVec = pair.second;

			calls.emplace_back(ProgramDrawCalls{ ProgramStore::Instance().GetProgram(progName), callsVec });
		}
		PROFILE_POP_WITH(g_Engine->Resources.Profile);


		PROFILE_PUSH_WITH(g_Engine->Resources.Profile, "Executing Draw Calls");
		for (auto& program_tmp : calls)
		{
			auto& program = program_tmp.Program;
			if (!program)
				continue;

			program->SetActive();

			for (auto& drawcall_tmp : program_tmp.DrawCalls)
			{
				auto p = PROFILE_EVENT_WITH(g_Engine->Resources.Profile, "Individual DrawCall", true);

				auto& drawcall = drawcall_tmp;

				if (!drawcall.Geometry)
					continue;

				auto& storage = drawcall.Geometry->GetStorage();
				if (!storage.Buffer)
					continue;

				storage.Buffer->UpdateIfDirty();

				MeshOffsetData offsetData;
				if (!storage.Buffer->TryGetMeshOffset(storage.ID, &offsetData))
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

	void DrawCallRenderer::UpdateLights(Matrixy4x4 view)
	{
		for (int i = 0; i < _lights.size(); ++i)
		{
			auto& l = _lights[i];
			l.DirectionVS = view.TransformNormal(l.DirectionWS);
			l.PositionVS = view.Transform(l.PositionWS);
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

	void DrawCallRenderer::UpdateMaterial(Program& prog, const Material& mat)
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
			auto it = mat.Textures.find(mapping.MatName);
			if (it != mat.Textures.end())
			{
				auto tex = it->second.GetTexture();
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

		GLuint out;
		glGenBuffers(1, &out);

		glBindBufferBase(GL_UNIFORM_BUFFER, LightBufBinding, out);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * LightCount, _lights.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return out;
	}

	DrawCallRenderer::DrawCallRenderer(CommonResources* resources)
		: _perObjectBuffer(InitPerObjectBuffer())
		, _lightBuffer(InitLightBuffer())
	{
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
		if (it == _drawCalls.end())
			return false;

		_drawCalls[key] = std::move(call);
		return true;
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
		return true;
	}

	bool DrawCallRenderer::RemoveDrawCall(const DrawCallReference& reference)
	{
		return RemoveDrawCall(reference.GetKey());
	}
}
