#include "MeshHelper.h"

namespace MeshHelp
{
	constexpr float approx_epsilon = 0.0001f;

	template<>
	bool Approximately<floaty3>(const floaty3& a, const floaty3& b)
	{
		return a.x > b.x - approx_epsilon && a.x < b.x + approx_epsilon
			&& a.y > b.y - approx_epsilon && a.y < b.y + approx_epsilon
			&& a.z > b.z - approx_epsilon && a.z < b.z + approx_epsilon;
	}

	template<>
	bool Approximately<Drawing::Full3DVertex>(const Drawing::Full3DVertex& a, const Drawing::Full3DVertex& b)
	{
		return Approximately(a.Position, b.Position)
			&& Approximately(a.Normal, b.Normal);
	}

	template<>
	bool Approximately<Drawing::VoxelVertex>(const Drawing::VoxelVertex& a, const Drawing::VoxelVertex& b)
	{
		return Approximately(a.Position, b.Position)
			&& Approximately(a.Normal, b.Normal);
	}

	template bool Approximately<floaty3>(const floaty3& a, const floaty3& b);
	template bool Approximately<Drawing::Full3DVertex>(const Drawing::Full3DVertex& a, const Drawing::Full3DVertex& b);
	template bool Approximately<Drawing::VoxelVertex>(const Drawing::VoxelVertex& a, const Drawing::VoxelVertex& b);
}