#include "VoxelChunkCuller.h"

Voxel::BlockCuller::BlockCuller(size_t x, size_t y, size_t z, ChunkyFrustumCuller *chunkboi) : m_X(x), m_Y(y), m_Z(z), m_ChunkyBoi(chunkboi)
{
}

bool Voxel::BlockCuller::InsideFrustum(const CameraFrustum &f)
{
	return m_ChunkyBoi->InFrustum(m_X, m_Y, m_Z, f);
}

Voxel::ChunkyFrustumCuller::ChunkyFrustumCuller(floaty3 origin, floaty3 extents) : m_ChunkSphere(GenerateSphere(origin, extents)), m_Origin(origin)
{
}

bool Voxel::ChunkyFrustumCuller::InFrustum(size_t x, size_t y, size_t z, const CameraFrustum &frustum)
{
	if (!m_InFrustum)
		return false;

	m_InFrustum = ChunkInFrustum(frustum);

	if (!m_InFrustum)
		return false;

	Sphere block_sphere;

	block_sphere.Centre.x = m_Origin.x + ((float)x + 0.5f) * Voxel::BlockSize;
	block_sphere.Centre.y = m_Origin.y + ((float)y + 0.5f) * Voxel::BlockSize;
	block_sphere.Centre.z = m_Origin.z + ((float)z + 0.5f) * Voxel::BlockSize;
	block_sphere.Radius = Voxel::Block_Radius;

	return InsideFrustum(frustum, block_sphere);
}

void Voxel::ChunkyFrustumCuller::Flush()
{
	m_InFrustum = true;
}

Voxel::Sphere Voxel::ChunkyFrustumCuller::GenerateSphere(floaty3 origin, floaty3 extents)
{
	Sphere out;

	out.Centre =
	{
		origin.x + (float)Voxel::Chunk_Size * 0.5f * Voxel::BlockSize,
		origin.y + (float)Voxel::Chunk_Height * 0.5f * Voxel::BlockSize,
		origin.z + (float)Voxel::Chunk_Size * 0.5f * Voxel::BlockSize
	};

	out.Radius = floaty3{ Voxel::Chunk_Size * Voxel::BlockSize, Voxel::Chunk_Height * Voxel::BlockSize, Voxel::Chunk_Size * Voxel::BlockSize }.magnitude();

	return out;
}

bool Voxel::ChunkyFrustumCuller::ChunkInFrustum(const CameraFrustum &f)
{
	return InsideFrustum(f, m_ChunkSphere);
}
