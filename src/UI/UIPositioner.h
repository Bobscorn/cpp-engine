#pragma once

#include <memory>

#include "Helpers/VectorHelper.h"

#include "Helpers/TransferHelper.h"

namespace UIPositioner
{
	/// <summary> Interface used by UI objects with a few matrices and the pure virtual Within(floaty2) method </summary>
	/// <remarks>
	/// Not required to be used
	/// </remarks>
	struct IPositionInfo
	{
	protected:
		Matrixy2x3 LocalToParent{ Matrixy2x3::Identity() }; // From Parent To Here Matrix, may be modified by parents
		Matrixy2x3 LocalToWorld{ Matrixy2x3::Identity() }; // Full Matrix Going through all Parents
		Matrixy2x3 WorldToLocal{ Matrixy2x3::Identity() }; // Inverse of CachedToHereTransform

		Matrixy2x3 ChildToLocal{ Matrixy2x3::Identity() }; // An extra matrix used to modify children space without modifying local space (can technically be ignored)
		Matrixy2x3 ChildToWorld{ Matrixy2x3::Identity() }; // Full Matrix Going through all Parents, includes ChildModify matrix

	public:		
		inline void ComputeWorldMatrix(Matrixy2x3 toparent)
		{
			LocalToWorld = LocalToParent * toparent;
			ChildToWorld = ChildToLocal * LocalToWorld;
			if (LocalToWorld.IsInvertible())
			{
				WorldToLocal = LocalToWorld;
				WorldToLocal.Invert();
			}
			else
			{
				WorldToLocal = Matrixy2x3::Scale({ 0.f, 0.f }); // Blank Matrix
			}
			MatrixChanged();
		}

		inline void SetHereToParent(Matrixy2x3 mat)
		{
			LocalToParent = mat;
		}

		inline Matrixy2x3 GetToWorldTrans()
		{
			return LocalToWorld;
		}

		inline Matrixy2x3 GetToHereChildTrans()
		{
			return ChildToWorld;
		}

		inline Matrixy2x3 GetChildMatrix()
		{
			return ChildToLocal;
		}
		
		inline Matrixy2x3 GetParentToHereMatrix()
		{
			return LocalToParent;
		}

		inline Matrixy2x3 GetFullMatrix()
		{
			return LocalToWorld;
		}

		virtual bool Within(floaty2 point) = 0;
		virtual void MatrixChanged() = 0;
		virtual Recty GetBoundingBox() = 0;
		virtual void RecommendBoundingBox(Recty parentspace) = 0; // Tells this to fit inside given (parent space) box, does not update matrices
	};

	struct RectangleInfo : virtual IPositionInfo
	{
		RectangleInfo(Recty bounds = { 0.f, 0.f, 25.f, 25.f }) : LocalBounds(bounds) {}

		bool Within(floaty2 point) override;
		void MatrixChanged() override;
		Recty GetBoundingBox() override;
		
		void RecommendBoundingBox(Recty parentspace) override;
		void DebugDraw(CommonResources *resources);

		inline Recty GetLocalBounds() const noexcept { return LocalBounds; }
	protected:
		Recty LocalBounds;
	};

	struct CircleInfo : IPositionInfo
	{
		float Radius = 1.f;

		bool Within(floaty2 point) override;
		void MatrixChanged() override;
		inline Recty GetBoundingBox() override { return { TransformedCenter.x - Radius, TransformedCenter.y - Radius, TransformedCenter.x + Radius, TransformedCenter.y + Radius }; }
		void RecommendBoundingBox(Recty box) override;

	private:
		floaty2 TransformedCenter;
	};
}