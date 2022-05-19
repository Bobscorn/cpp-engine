#pragma once

#include "Helpers/VectorHelper.h"

#include "Material.h"

#include <cstdint>

namespace Drawing
{
	namespace Materials
	{
		struct Material3DStandard
		{
			constexpr Material3DStandard() :
				AmbientColor(0.2f, 0.2f, 0.2f, 1.0f),
				EmissiveColor(0.f, 0.f, 0.f, 1.0f),
				DiffuseColor(0.5f, 0.5f, 0.5f, 1.0f),
				SpecularColor(0.75f, 0.75f, 0.7f, 12.0f),
				IndexOfRefraction(1.0f),
				TextureBitmap(0x0),
				Padding(0.f, 0.f)
			{
			}

			constexpr Material3DStandard(floaty4 Ambient, floaty4 Emissive, floaty4 Diffuse, floaty4 Specular, float IOR, UINT TextureBitmask) :
				AmbientColor(Ambient),
				EmissiveColor(Emissive),
				DiffuseColor(Diffuse),
				SpecularColor(Specular),
				IndexOfRefraction(IOR),
				TextureBitmask(TextureBitmask),
				Padding(0.f, 0.f)
			{
			}

			// 0 bytes
			floaty4 AmbientColor;
			// 16 bytes
			floaty4 EmissiveColor;
			// 32 bytes
			floaty4 DiffuseColor;
			// 48 bytes
			floaty4 SpecularColor;
			// 64 bytes

			float IndexOfRefraction;

			// Bit mask:
			// 0x1 1: Ambient
			// 0x2 2: Emissize
			// 0x4 3: Diffuse
			// 0x8 4: Specular
			// 0x10 5: Normal
			// 0x20 6: Bump
			// 0x40 7: Opacity
			uint32_t TextureBitmask; // Set to indicate which textures are valid

			floaty2 padding;
			// 80 bytes
		}; // 80 byte material padded to be a multiple of 16
	}
}