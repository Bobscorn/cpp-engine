#pragma once

#include "Helpers/DrawingHelper.h"

#include "Graphics3D.h"
#include "Drawing/DrawCallReference.h"
#include "Drawing/DrawCall.h"
#include "Frustum.h"

struct IRen3D
{
	virtual ~IRen3D() {}

	virtual void SetLight(unsigned int Index, Light light) = 0;
	[[nodiscard]] virtual const Light *GetLight(unsigned int Index) const = 0;
	[[nodiscard]] virtual Light *GetLight(unsigned int index) = 0;

	[[nodiscard]] virtual Drawing::DrawCall CreateDrawCall(std::vector<FullVertex> vertices, std::vector<unsigned int> indices, Material mat, Matrixy4x4 *matrix = nullptr) = 0;
	[[nodiscard]] virtual Drawing::DrawCall CreateDrawCall(std::vector<FullVertex> vertices, std::vector<unsigned int> indices, Matrixy4x4 *matrix = nullptr) = 0;
	[[nodiscard]] virtual size_t AddDrawCall(Drawing::DrawCall call) = 0;
	virtual void RemoveDrawCall(size_t Key) = 0;
	[[nodiscard]] virtual Drawing::DrawCall *GetDrawCall(size_t key) = 0;

	[[nodiscard]] virtual std::shared_ptr<GeoThing> AddGeometry(std::vector<FullVertex> vertices, std::vector<unsigned int> indices) = 0;
	virtual void RemoveGeometry(size_t key) = 0;

	virtual void DrawCalls(Matrixy4x4 View, Matrixy4x4 Proj = Matrixy4x4::Identity(), Voxel::CameraFrustum frustum = Voxel::CameraFrustum()) = 0;

	virtual void DidSomething() = 0;
};

namespace Drawing
{
	// Temporary interface until GLRenv2 is properly finished
	struct IRen3Dv2
	{
		virtual ~IRen3Dv2() {}

		[[nodiscard]] virtual Drawing::DrawCallReference SubmitDrawCall(DrawCallv2 newVal) = 0;
		virtual bool RemoveDrawCall(size_t key) = 0;
		[[nodiscard]] virtual const Drawing::DrawCallv2* GetDrawCall(size_t key) const = 0;
		virtual bool SetDrawCall(size_t key, DrawCallv2 newVal) = 0; // Returns whether this key already existed

		virtual void SetLight(size_t Index, Light light) = 0;
		[[nodiscard]] virtual const Light *GetLight(size_t Index) const = 0;
		[[nodiscard]] virtual Light *GetLight(size_t index) = 0;
	};
}