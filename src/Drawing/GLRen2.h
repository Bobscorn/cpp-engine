#pragma once

#include <unordered_map>

#include "DrawCall.h"
#include "DrawCallReference.h"
#include "Frustum.h"
#include "IRen3D.h"

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
		static constexpr size_t LightCount = LIGHT_COUNT;

	private:
		static GLuint LightBufBinding;
		static GLuint PerObjectBufBinding;
		static GLuint MaterialBufBinding;

		std::unordered_map<size_t, DrawCallv2> _drawCalls;
		std::array<Light, LightCount> _lights;

		GLBuffer _perObjectBuffer;
		GLBuffer _lightBuffer;

		size_t _nextKey = 1;

		void UpdateLights(Matrixy4x4 view);

		void UpdatePerObject(Matrixy4x4 world, Matrixy4x4 view, Matrixy4x4 proj);
		void UpdateMaterial(Program& prog, const Material& mat);
		void UpdateTextures(Program& prog, const Material& mat);


		GLuint InitPerObjectBuffer();
		GLuint InitLightBuffer();

	public:
		DrawCallRenderer(CommonResources* resources);

		static GLuint GetLightBufBinding();
		static GLuint GetPerObjectBufBinding();
		static GLuint GetMaterialBufBinding();

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