#pragma once

#include "Helpers/VectorHelper.h"
#include "Helpers/DrawingHelper.h"
#include "Helpers/BulletHelper.h"

#include "Game/VoxelStuff/VoxelAbility.h"
#include "Drawing/Graphics3D.h"
#include "Structure/ShapeStore.h"

#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btMotionState.h>

#include <memory>

namespace Voxel
{
	struct SphereCuller : Drawing::ICuller
	{
		SphereCuller(Matrixy4x4 *trans, float rad);

		bool InsideFrustum(const CameraFrustum &f) override;

		void UpdateRadius(float rad);
	protected:
		Matrixy4x4 *m_Trans;
		float m_Radius;
	};

	namespace Entities
	{
		// Needs to be an IShape to work easily with physics
		struct Ball : Voxel::Entity, btMotionState, FullResourceHolder, BulletHelp::INothingInterface
		{
			Ball(VoxelWorld *world, CommonResources *resources, floaty3 position);
			~Ball();

			floaty3 GetCentre() override;

			virtual void BeforeDraw() override;
			virtual void AfterDraw() override;

			virtual bool HitByProjectile(Projectile *proj, floaty3 hit_point) override;
			virtual void TakeKnockbackDirect(floaty3 knock_back) override;

			void getWorldTransform(btTransform &out) const override;
			void setWorldTransform(const btTransform &in) override;

		protected:

			void GenerateBody();
			size_t GenerateDrawcall();

			Matrixy4x4 m_Trans;

			std::shared_ptr<btSphereShape> m_Shape;
			std::shared_ptr<btRigidBody> m_Body;

			SphereCuller m_Cull;
			
			size_t m_DrawKey = 0;
		};
	}
}