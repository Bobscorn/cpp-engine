#include "ParkourBlocks.h"

#include <Structure/BasicShapes.h>
#include <Game/VoxelStuff/VoxelWorld.h>

Parkour::ParkourLightBlock::ParkourLightBlock(
	G1::IGSpace* container, 
	CommonResources* resources, 
	Voxel::VoxelWorld* world, 
	Voxel::VoxelChunk* chunk, 
	Voxel::ChunkBlockCoord pos)
	: FullResourceHolder(resources)
	, IShape(container, "Parkour Light Block")
	, ICube(world, chunk, pos)
{

}

void Parkour::ParkourLightBlock::OnLoaded()
{
	try
	{
		auto pos = m_World->GetPhysPosFromBlockCoord(Voxel::BlockCoord{ m_Chunk->GetCoord(), m_Pos });

		Light light;
		light.Color = floaty3{ 1.f, 1.f, 1.f };
		light.Intensity = 3.5f;
		light.Attenuation = floaty3{ 2.f, 0.1f, 0.15f };
		light.Enabled = true;
		light.PositionWS = floaty4{ pos, 1.f };
		light.PositionVS = floaty4{};
		light.DirectionWS = floaty4{ 0.f, -1.f, 0.f, 1.f };
		light.DirectionVS = floaty4{};
		light.Range = 50.f;
		light.Type = LIGHT_SPOT;
		light.SpotlightAngle = Math::DegToRadF * 35.f;
		light.Padding = 0.f;
		this->AddChild<G1I::LightShape>("Parkour Light", light);
	}
	catch (G1I::NoFreeLightException& e)
	{
	}
}

void Parkour::ParkourLightBlock::OnUnloaded()
{
	for (int i = children.size(); i-- > 0; )
	{
		if (auto child = dynamic_cast<G1I::LightShape*>(children[i].get()); child)
		{
			std::swap(children[i], children.back());
			children.pop_back();
		}
	}
}

std::unique_ptr<Voxel::ICube> Parkour::ParkourLightBlock::Clone(Voxel::VoxelWorld* world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos) const
{
	return std::make_unique<ParkourLightBlock>(GetContainer(), mResources, world, chunk, pos);
}

