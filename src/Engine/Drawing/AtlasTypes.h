#pragma once

#include <string>
#include <array>

namespace Voxel
{

	enum class AtlasType
	{
		DIFFUSE = 0,
		SPECULAR = 1,
		EMISSIVE = 2, // Emissive and Ambient effectively do the same thing and have been combined
		AMBIENT = 2,
		NORMAL = 3,
		BUMP = 4,
		OTHER = 5,
	};

	struct AtlasTextureName
	{
		std::string atlasName;
		AtlasType atlasType;
	};

	constexpr std::array<AtlasType, 5> AtlasTypes{ AtlasType::DIFFUSE, AtlasType::SPECULAR, AtlasType::EMISSIVE, AtlasType::NORMAL, AtlasType::BUMP };
}