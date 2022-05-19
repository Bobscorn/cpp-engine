#pragma once

#include <unordered_map>

#include "DrawCall.h"
#include "DrawCallReference.h"
#include "Frustum.h"

#include "Helpers/VectorHelper.h"

namespace Drawing
{
	struct DefaultPerObjectStruct
	{
		Matrixy4x4 WorldView;
		Matrixy4x4 WorldViewProj;
	};

	class DrawCallRenderer
	{
		std::unordered_map<size_t, DrawCallv2> _drawCalls;

		GLBuffer _perObjectBuffer;

		size_t _nextKey = 1;

		void UpdateLights(Matrixy4x4 view);

		void UpdatePerObject(Matrixy4x4 worldView, Matrixy4x4 worldViewProj);
		void UpdateMaterial(Program& prog, const Material& mat);


		GLuint InitPerObjectBuffer();

	public:
		DrawCallRenderer(CommonResources* resources);

		static constexpr GLuint MaterialBufLoc = 1u;
		static constexpr GLuint PerObjectBufLoc = 0u;
		static constexpr GLuint LightBufLoc = 7u;

		DrawCallReference SubmitDrawCall(DrawCallv2 drawCall);

		const DrawCallv2 *GetDrawCall(size_t key) const;
		const DrawCallv2 *GetDrawCall(const DrawCallReference& reference) const;
		bool SetDrawCall(size_t key, DrawCallv2 call); // Returns true if it updated an existing value, false if it is a new value
		bool SetDrawCall(const DrawCallReference& reference, DrawCallv2 call); // Returns true if it updated an existing value, false if it is a new value

		bool RemoveDrawCall(size_t key); // Returns whether the key existed
		bool RemoveDrawCall(const DrawCallReference& reference); // Returns whether the key existed

		void Draw(Matrixy4x4 View, Matrixy4x4 Proj, Voxel::CameraFrustum frustum);
	};
}