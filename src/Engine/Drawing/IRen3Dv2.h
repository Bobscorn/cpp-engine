#pragma once

#include "Graphics3D.h"
#include "DrawCall.h"
#include "DrawCallReference.h"

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
		[[nodiscard]] virtual const Light* GetLight(size_t Index) const = 0;
		[[nodiscard]] virtual Light* GetLight(size_t index) = 0;
	};
}