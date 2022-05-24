#include "GLRen2.h"

#include <utility>

#include "Helpers/VectorHelper.h"

#include "Program.h"

namespace Drawing
{
	struct ProgramDrawCalls
	{
		std::shared_ptr<Program> Program;
		std::vector<DrawCallv2> DrawCalls;
	};

	void DrawCallRenderer::Draw(Matrixy4x4 View, Matrixy4x4 Proj, Voxel::CameraFrustum frustum)
	{
		Matrixy4x4 ViewProj = Matrixy4x4::Multiply(Proj, View);
		Matrixy4x4 WorldView = Matrixy4x4::Identity();
		Matrixy4x4 WorldViewProj = Matrixy4x4::Identity();

		GLuint lastVertexBuffer = 0;

		UpdateLights(View);

		CHECK_GL_ERR("Pre-Enabling Depth Test");
		glEnable(GL_DEPTH_TEST);
		CHECK_GL_ERR("Enabling Depth Test");

		// Temporarily we will not sort or cull calls
		// auto culled_calls = Cull(calls); Cull here maybe?

		// Also temporarily we will do a super slow and iterate the drawcalls every frame and collect them into a vector
		std::vector<ProgramDrawCalls> calls;

		// Part of temporary-ness, use unordered map to gather all the calls into per-program vectors
		std::unordered_map<std::string, std::vector<DrawCallv2>> programCallMap{};

		for (auto& pair : _drawCalls)
		{
			auto& call = pair.second;
			if (!call.Material)
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

		glBindBufferBase(GL_UNIFORM_BUFFER, PerObjectBufLoc, _perObjectBuffer.Get());
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		for (auto& program_tmp : calls)
		{
			auto& program = program_tmp.Program;
			if (!program)
				continue;

			program->SetActive();

			for (auto& drawcall_tmp : program_tmp.DrawCalls)
			{
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

				WorldView = Matrixy4x4::Multiply(View, (*drawcall.Matrix));
				WorldViewProj = Matrixy4x4::Multiply(ViewProj, (*drawcall.Matrix));

				UpdatePerObject(WorldView, WorldViewProj);
				UpdateMaterial(*program, *drawcall.Material);

					
				program->BindVAO();

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawcall.Geometry->GetStorage().Buffer->GetIBO().Get());

				glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)offsetData.IndicesCount, GL_UNSIGNED_INT, (GLvoid*)(offsetData.IndexStart * sizeof(GLuint)), (GLint)offsetData.IndexOffset);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				glBindVertexArray(0);

			}
		}
	}

	void DrawCallRenderer::UpdateLights(Matrixy4x4 view)
	{
	}

	void DrawCallRenderer::UpdatePerObject(Matrixy4x4 worldView, Matrixy4x4 worldViewProj)
	{
		DefaultPerObjectStruct data;
		data.WorldView = worldView;
		data.WorldViewProj = worldViewProj;

		glBindBuffer(GL_UNIFORM_BUFFER, _perObjectBuffer.Get());
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DefaultPerObjectStruct), (GLvoid*)&data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void DrawCallRenderer::UpdateMaterial(Program& prog, const Material& mat)
	{
		prog.SetMaterial(mat);
	}

	GLuint DrawCallRenderer::InitPerObjectBuffer()
	{
		CHECK_GL_ERR("Pre Bind/init per object buffer");
		GLuint out = 0;

		glGenBuffers(1, &out);

		glBindBufferBase(GL_UNIFORM_BUFFER, PerObjectBufLoc, out);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(DefaultPerObjectStruct), nullptr, GL_STREAM_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		CHECK_GL_ERR("Post Bind and init PerObject Buffer");

		return out;
	}

	DrawCallRenderer::DrawCallRenderer(CommonResources* resources)
		: _perObjectBuffer(InitPerObjectBuffer())
	{
	}

	DrawCallReference DrawCallRenderer::SubmitDrawCall(DrawCallv2&& drawCall)
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

	bool DrawCallRenderer::SetDrawCall(size_t key, DrawCallv2&& call)
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
