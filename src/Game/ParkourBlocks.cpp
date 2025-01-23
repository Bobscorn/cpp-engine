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
		DINFO("Loading light shape");
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
		light.ShadowIndex = 0;
		this->AddChild<G1I::LightShape>("Parkour Light", light);
	}
	catch (G1I::NoFreeLightException& e)
	{
		(void)e;
		DINFO("No lights available!");
	}
}

void Parkour::ParkourLightBlock::OnUnloaded()
{
	DINFO("Unloading light shape");
	for (size_t i = children.size(); i-- > 0; )
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

Parkour::ParkourCheckpointBlock::ParkourCheckpointBlock(G1::IGSpace* container, CommonResources* resources, Voxel::VoxelWorld* world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos)
	: FullResourceHolder(resources)
	, G1::IShape(container, "Parkour Checkpoint Block")
	, Voxel::ICube(world, chunk, pos)
	, m_Player()
	, m_Tracker()
{
}

void Parkour::ParkourCheckpointBlock::AfterDraw()
{
	if (!m_Player)
	{
		m_Player = this->Container->FindShapeFPtr("Puzzle Controller").SketchyCopy<Voxel::VoxelPlayer>();
	}

	if (!m_Tracker)
	{
		m_Tracker = this->Container->FindShapeFPtr("Parkour Player Tracker").SketchyCopy<PlayerTrackerShape>();
	}

	if (!m_Player)
	{
		DERROR("Could not find Player object!");
		return;
	}
	if (!m_Tracker)
	{
		DERROR("Could not find Tracker object!");
		return;
	}

	auto myPos = m_World->GetPhysPosOfBlock(this);
	auto airAboveCheckpoint = myPos + floaty3{ 0.f, 1.5f, 0.f };

	if (m_Tracker->GetRespawnPosition() && *m_Tracker->GetRespawnPosition() == airAboveCheckpoint)
		return;

	auto playerPos = m_Player->GetPosition();

	if (airAboveCheckpoint.manhattan_distance(playerPos) < 2.f)
	{
		DINFO(std::string("Checkpoint set: (") + std::to_string(airAboveCheckpoint.x) + ", " + std::to_string(airAboveCheckpoint.y) + ", " + std::to_string(airAboveCheckpoint.z) + ")");
		m_Tracker->SetRespawnPosition(airAboveCheckpoint);
	}
}

std::unique_ptr<Voxel::ICube> Parkour::ParkourCheckpointBlock::Clone(Voxel::VoxelWorld* world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos) const
{
	return std::make_unique<ParkourCheckpointBlock>(Container, mResources, world, chunk, pos);
}
