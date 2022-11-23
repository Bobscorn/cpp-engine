#pragma once

#include <Drawing/Graphics1.h>
#include <Game/VoxelStuff/VoxelPlayer.h>

#include <Structure/BasicShapes.h>

namespace Parkour
{


	class ParkourTorchShape : public virtual G1::IShape
	{
	public:
		ParkourTorchShape(G1::IShapeThings things, Pointer::f_ptr<Voxel::VoxelPlayer> playerShape);

		virtual void BeforeDraw() override;
		inline virtual void AfterDraw() override {}
	protected:
		Pointer::f_ptr<Voxel::VoxelPlayer> m_PlayerShape;

		Pointer::f_ptr<G1I::LightShape> m_LightShape;

	};
}
