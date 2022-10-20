#include "ParkourShapes.h"

Parkour::ParkourLevelShape::ParkourLevelShape(G1::IShapeThings things, ParkourLevel level)
	: IShape(things)
	, FullResourceHolder(things.Resources)
	, m_Level(level)
{
}

void Parkour::ParkourLevelShape::BeforeDraw()
{
}

void Parkour::ParkourLevelShape::AfterDraw()
{
}

const Parkour::ParkourLevel& Parkour::ParkourLevelShape::GetLevelData() const
{
	return m_Level;
}

void Parkour::ParkourLevelShape::SetLevelData(ParkourLevel level)
{
	m_Level = level;
}

Parkour::ParkourGeneratorShape::ParkourGeneratorShape(G1::IShapeThings things, Pointer::f_ptr<ParkourLevelShape> levelShape, Pointer::f_ptr<Voxel::VoxelWorld> worldShape)
	: IShape(things)
	, FullResourceHolder(things.Resources)
	, m_LevelShape(std::move(levelShape))
	, m_WorldShape(std::move(worldShape))
{
	Generate();
}

void Parkour::ParkourGeneratorShape::Generate()
{
	auto levelDat = m_LevelShape->GetLevelData();
	DINFO("Parkour: Generating '" + std::to_string(levelDat.NumGeneratedSteps) + "' parkour steps...");

	ParkourGenerationInfo info;
	info.NumGeneratedSteps = levelDat.NumGeneratedSteps;
	info.StartDirection = levelDat.StartFace;
	info.StartPosition = Parkour::BlockCoordTointy3(levelDat.StartPos);
	info.UsableSteps = levelDat.UsableSteps;
	info.WeightedAltitudes = levelDat.WeightedAltitudes;
	info.WeightedDistances = levelDat.WeightedDistances;
	
	auto gen = Parkour::GenerateParkour(info);
	DINFO("Parkour: Generated '" + std::to_string(gen.Blocks.size()) + "' blocks");
	levelDat.EndPosition = Parkour::inty3ToBlockCoord(gen.EndPosition);
	levelDat.EndFace = gen.EndDirection;
	levelDat.GoalPosition = Parkour::inty3ToBlockCoord(gen.EndPosition - Voxel::BlockFaceHelper::GetDirectionI(gen.EndDirection) + Vector::inty3(0, 2, 0));
	m_LevelShape->SetLevelData(levelDat);

	for (auto& block : gen.Blocks)
	{
		auto realPos = Parkour::inty3ToBlockCoord(block.first);
		
		m_WorldShape->ReplaceStaticCube(realPos, block.second);
	}
}

Parkour::PlayerTrackerShape::PlayerTrackerShape(G1::IShapeThings things, Parkour::PlayerTrackingData data)
	: IShape(things)
	, FullResourceHolder(things.Resources)
	, m_Data(std::move(data))
{
}

void Parkour::PlayerTrackerShape::BeforeDraw()
{
	constexpr float GoalDistanceSq = 1.f;

	// check if near end
    auto diff = m_Data.Player->GetPosition() - m_Data.World->GetPhysPosFromBlockCoord(m_Data.Level->GetLevelData().GoalPosition);
	if (diff.mag2() < GoalDistanceSq)
	{
		DINFO("Parkour: Player is within winning area!");
		// Player win
	}

	if (m_Data.Player->GetPosition().y < m_Data.MinimumY)
	{
		// Player fell
		// Tp them back to start
		auto newPos = m_Data.World->GetPhysPosFromBlockCoord(m_Data.Level->GetLevelData().StartPos) + floaty3{ 0.f, 1.5f, 0.f };
		DINFO("Parkour: Resetting player to: floaty3(" + std::to_string(newPos.x) + ", " + std::to_string(newPos.y) + ", " + std::to_string(newPos.z) + ")");
		m_Data.Player->SetPosition(newPos);
		m_Data.Player->SetVelocity({ 0.f, 0.f, 0.f });
	}
}

void Parkour::PlayerTrackerShape::AfterDraw() {}

Parkour::ParkourEndShape::ParkourEndShape(G1::IShapeThings things, Pointer::f_ptr<ParkourLevelShape> levelShape, Pointer::f_ptr<Voxel::VoxelWorld> worldShape)
	: IShape(things)
	, FullResourceHolder(things.Resources)
	, m_LevelShape(levelShape)
	, m_WorldShape(worldShape)
	, m_DrawTrans(std::make_shared<Matrixy4x4>(Matrixy4x4::Identity()))
	, m_DrawCall(mResources->Ren3v2->SubmitDrawCall(Drawing::DrawCallv2("default-cube", "default-3d-material", m_DrawTrans, "Parkour End Object", true)))
	, m_RotationDegreesPerSec(45.f)
	, m_RotationDegrees(0.f)
{
}

void Parkour::ParkourEndShape::BeforeDraw()
{
	m_RotationDegrees += m_RotationDegreesPerSec * (float)(*mResources->DeltaTime);

	Matrixy4x4 trans = Matrixy4x4::Scale(0.5f);
	trans = Matrixy4x4::MultiplyE(trans, Matrixy4x4::RotationAxisD(floaty3{ 0.f, 1.f, 0.f }, m_RotationDegrees));
	auto endPos = m_WorldShape->GetPhysPosFromBlockCoord(m_LevelShape->GetLevelData().EndPosition);
	trans = Matrixy4x4::MultiplyE(trans, Matrixy4x4::Translate(endPos));

	*m_DrawTrans = trans;
}
