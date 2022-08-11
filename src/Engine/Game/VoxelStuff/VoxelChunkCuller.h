#pragma once

#include "VoxelValues.h"

#include "Drawing/Frustum.h"
#include "Helpers/DrawingHelper.h"

namespace Voxel
{
	struct ChunkyFrustumCuller
	{
		ChunkyFrustumCuller(floaty3 origin, floaty3 extents);
		~ChunkyFrustumCuller() {};

		bool InFrustum(size_t x, size_t y, size_t z, const CameraFrustum &frustum);

		void Flush();

	private:

		Sphere GenerateSphere(floaty3 origin, floaty3 extents);

		bool ChunkInFrustum(const CameraFrustum &f);

		floaty3 m_Origin;
		Sphere m_ChunkSphere;
		bool m_InFrustum = true;
	};

	struct BlockCuller : Drawing::ICuller
	{
		BlockCuller(size_t x, size_t y, size_t z, ChunkyFrustumCuller *chunkboi);


		bool InsideFrustum(const CameraFrustum &f) override;

	private:
		ChunkyFrustumCuller *m_ChunkyBoi;
		size_t m_X, m_Y, m_Z;
	};
}