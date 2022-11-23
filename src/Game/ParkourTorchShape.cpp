#include "ParkourTorchShape.h"

Parkour::ParkourTorchShape::ParkourTorchShape(G1::IShapeThings things, Pointer::f_ptr<Voxel::VoxelPlayer> playerShape)
	: FullResourceHolder(things.Resources)
	, IShape(things)
	, m_PlayerShape(std::move(playerShape))
{
	Light light;
	light.Color = floaty3{ 1.f, 1.f, 1.f };
	light.Intensity = 5.f;
	light.Attenuation = floaty3{ 2.f, 0.25f, 0.05f };
	light.Enabled = true;
	light.PositionWS = floaty4{ 0.f, 3.f, 0.f, 1.f };
	light.PositionVS = floaty4{};
	light.DirectionWS = floaty4{ 0.f, -1.f, 0.f, 1.f };
	light.DirectionVS = floaty4{};
	light.Range = 50.f;
	light.Type = LIGHT_SPOT;
	light.SpotlightAngle = Math::DegToRadF * 30.f;
	light.Padding = 0.f;
	m_LightShape = this->AddChild<G1I::LightShape>("Player Torch Light", light);
}

void Parkour::ParkourTorchShape::BeforeDraw()
{
	auto* light = m_LightShape->GetLight();

	light->PositionWS = floaty4{ m_PlayerShape->GetPosition(), 1.f };
	light->DirectionWS = floaty4{ m_PlayerShape->GetOrientation(), 1.f};
}
