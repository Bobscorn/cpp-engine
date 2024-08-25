#pragma once

#include <memory>

#include <Drawing/Graphics1.h>
#include <Drawing/DrawCallReference.h>
#include <Drawing/Mesh.h>

#include <Game/VoxelStuff/VoxelCube.h>

namespace Parkour
{
	class ParkourSignShape : public Voxel::ICube
	{
	public:
		ParkourSignShape(G1::IShapeThings things, Voxel::VoxelWorld* world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos, std::shared_ptr<Drawing::Mesh> sign_mesh = nullptr);

		virtual void OnLoaded() override;
		virtual void OnUnloaded() override;

		inline virtual void BeforeDraw() override {}
		inline virtual void AfterDraw() override {}

		virtual std::unique_ptr<ICube> Clone(Voxel::VoxelWorld* world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos) const override;

	protected:
		std::shared_ptr<Drawing::Mesh> m_SignTextMesh;
		Drawing::DrawCallReference m_SignTextDrawCall;

		static const std::string SignMeshName;
		static const std::string SignTextureName;
		static const std::string SignMaterialName;
	};
}

