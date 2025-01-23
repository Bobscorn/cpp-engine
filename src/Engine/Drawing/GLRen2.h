#pragma once

#include <unordered_map>
#include <functional>
#include <variant>

#include "DrawCall.h"
#include "DrawCallReference.h"
#include "Frustum.h"
#include "IRen3D.h"
#include "IRen3Dv2.h"
#include "FrameBuffer.h"

#include "Helpers/VectorHelper.h"

namespace Drawing
{
	struct DefaultPerObjectStruct
	{
		Matrixy4x4 World;
		Matrixy4x4 View;
		Matrixy4x4 Projection;
		Matrixy4x4 WorldView;
		Matrixy4x4 WorldViewProj;
	};

	class DrawCallRenderer : public IRen3Dv2
	{
	public:
		static constexpr size_t LightCount = LIGHT_COUNT;
		static constexpr size_t ShadowLightCount = MAX_SHADOW_LIGHT_COUNT;
		static constexpr size_t ShadowMapSize = 1024;
		static constexpr size_t CascadeCount = 3ull;

	private:
		static GLuint LightBufBinding;
		static GLuint PerObjectBufBinding;
		static GLuint MaterialBufBinding;

		std::unordered_map<size_t, DrawCallv2> _drawCalls;
		std::array<Light, LightCount> _lights;

		GLBuffer _perObjectBuffer;
		GLBuffer _lightBuffer;

		size_t _nextKey = 1;

		std::unordered_map<ProgramReference, std::vector<std::reference_wrapper<const DrawCallv2>>> m_DrawCallGroups;
		bool _drawCallsDirty = true;

		BufferUpdateMode m_bufferUpdateMode = BufferUpdateMode::MAP_WITH_INVALIDATE;

		void UpdateDrawCalls();

		void UpdateLights(Matrixy4x4 view);

		void UpdatePerObject(Matrixy4x4 world, Matrixy4x4 view, Matrixy4x4 proj);
		void UpdateMaterial(Program& prog, Material& mat);
		void UpdateTextures(Program& prog, const Material& mat);
		void UpdateShadowMaps(Program& prog);

		GLProgram _shadowProgram;
		GLFrameBuffer _shadowFBO;
		std::array<std::unique_ptr<GLImage>, ShadowLightCount> _shadowTextures;
		std::array<Matrixy4x4, ShadowLightCount> _shadowMatrices;

		std::array<float, CascadeCount + 1> _shadowCascadeDistances = { 0.1f, 10.f, 100.f, 150.f };
		std::array<GLImage, CascadeCount> _shadowCascadeTextures;
		std::array<Matrixy4x4, CascadeCount> _shadowCascadeMatrices;
		GLProgram CreateShadowProgram(const std::string& vertexShader, const std::string& fragmentShader);
		GLFrameBuffer CreateShadowFBO();
		void DrawShadows(Matrixy4x4 view, Matrixy4x4 proj);
		void DrawPointShadows(const Light& light, GLuint mapSize);
		void DrawSpotlightShadows(const Light& light, GLuint mapSize);
		void DrawSunShadows(const Light& light, Matrixy4x4 view, Matrixy4x4 proj, GLuint mapSize);
		void DrawShadowGeometry(Matrixy4x4 lightViewProj);


		GLuint InitPerObjectBuffer();
		GLuint InitLightBuffer();

	public:
		DrawCallRenderer(CommonResources* resources);

		static GLuint GetLightBufBinding();
		static GLuint GetPerObjectBufBinding();
		static GLuint GetMaterialBufBinding();
		inline static GLsizei GetShadowMatrixBufferSize() { return ShadowLightCount * sizeof(Matrixy4x4); }


		// Draw Calls v

		DrawCallReference SubmitDrawCall(DrawCallv2 drawCall) override;

		const DrawCallv2 *GetDrawCall(size_t key) const override;
		const DrawCallv2 *GetDrawCall(const DrawCallReference& reference) const;
		bool SetDrawCall(size_t key, DrawCallv2 call) override; // Returns true if it updated an existing value, false if it is a new value
		bool SetDrawCall(const DrawCallReference& reference, DrawCallv2 call); // Returns true if it updated an existing value, false if it is a new value

		bool RemoveDrawCall(size_t key) override; // Returns whether the key existed
		bool RemoveDrawCall(const DrawCallReference& reference); // Returns whether the key existed

		void Draw(Matrixy4x4 View, Matrixy4x4 Proj, Voxel::CameraFrustum frustum);

		// Lights v
		Light* GetLight(size_t index) override;
		const Light* GetLight(size_t index) const override;
		void SetLight(size_t index, Light val) override;
	};
}