#pragma once

#include <math.h>
#include <cstdint>

namespace Voxel
{
	constexpr float BlockSize = 0.8f;
	constexpr uint8_t Chunk_Size = 32u;
	constexpr uint8_t Chunk_Height = 48u;

	constexpr float Block_Radius = 1.224744871391589f;

	constexpr float Chunk_Width = (float)Chunk_Size * BlockSize;
	constexpr double Chunk_Width_Double = (double)Chunk_Size * (double)BlockSize;
	constexpr float Chunk_Tallness = (float)Chunk_Height * BlockSize;
	constexpr double Chunk_Tallness_Double = (double)Chunk_Height * (double)BlockSize;
}
