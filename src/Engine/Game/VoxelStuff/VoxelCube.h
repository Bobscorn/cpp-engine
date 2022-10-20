#pragma once

#include "Drawing/Graphics1.h"
#include "Drawing/Graphics3D.h"

#include "VoxelChunkCuller.h"
#include "VoxelTypes.h"

#include "Helpers/BulletHelper.h"
#include "Helpers/VectorHelper.h"

#include <memory>

namespace Voxel
{

	struct VoxelWorld;

	struct CubeCreationDescription
	{
		std::vector<std::string> MaterialNames;
		std::vector<std::string> GeometryNames;
	};

	struct CubeCreationStuff
	{
	};

	struct ICube : virtual G1::IShape
	{
		ICube(VoxelWorld *world, std::string name) : m_World(world), m_Name(name) {}

		// Position in displaced physics space
		virtual void BecomeDynamic(floaty3 pos) { (void)pos; };

		// Position in absolute world space
		virtual bool TryBecomeStatic(size_t x, size_t y, size_t z) { (void)x; (void)y; (void)z; return false; };

		virtual void UpdatePosition(floaty3 new_position, size_t new_x, size_t new_y, size_t new_z, ChunkyFrustumCuller *new_culler) = 0;

		inline VoxelWorld *GetWorld() const { return m_World; }
		inline const std::string& GetBlockName() const { return m_Name; }
		size_t GetBlockID() const;

		virtual std::unique_ptr<ICube> Clone() const = 0;
		inline virtual bool WantsUpdate() const { return false; }

	protected:
		VoxelWorld *m_World = nullptr;
		std::string m_Name = "unnamed";
	};

	struct VoxelCube : ICube, virtual G1::IShape, BulletHelp::INothingInterface
	{
		VoxelCube(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 position, size_t x, size_t y, size_t z, std::string name);
		~VoxelCube();

		virtual void BeforeDraw() override;
		virtual void AfterDraw() override;

		floaty3 GetPosition() const;

		void SetCull(ChunkyFrustumCuller *chunkyboi);
		void SetChunkPosition(size_t x, size_t y, size_t z);

		virtual void UpdatePosition(floaty3 new_position, size_t new_x, size_t new_y, size_t new_z, ChunkyFrustumCuller *new_culler) override;
		virtual std::unique_ptr<ICube> Clone() const override;

	protected:

		void ResetCuller();
		
		std::shared_ptr<btBoxShape> GetShape();
		std::shared_ptr<btCollisionObject> GetBody(btCollisionShape *shape);

		Matrixy4x4 m_Trans;
		size_t m_X = 0, m_Y = 0, m_Z = 0;

		//std::shared_ptr<btBoxShape> m_Shape;
		//static std::weak_ptr<btBoxShape> s_Shape;

		//std::shared_ptr<btCollisionObject> m_Body;
	};

	/*struct VoxelCubeKeptForTemplate : ICube, virtual G1::IShape, BulletHelp::INothingInterface
	{
		VoxelCubeKeptForTemplate(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 position, size_t x, size_t y, size_t z, ChunkyFrustumCuller *chunkcull = nullptr);
		~VoxelCubeKeptForTemplate();

		virtual void BeforeDraw() override;
		virtual void AfterDraw() override;

		virtual std::shared_ptr<GeoThing> GetGeo() override;

		floaty3 GetPosition() const;

		void SetCull(ChunkyFrustumCuller *chunkyboi);
		void SetChunkPosition(size_t x, size_t y, size_t z);

		virtual void UpdatePosition(floaty3 new_position, size_t new_x, size_t new_y, size_t new_z, ChunkyFrustumCuller *new_culler) override;

	protected:

		void ResetCuller();
		
		std::shared_ptr<Material> GetMat();
		std::shared_ptr<btBoxShape> GetShape();
		std::shared_ptr<btCollisionObject> GetBody(btCollisionShape *shape);

		Matrixy4x4 m_Trans;
		size_t m_X = 0, m_Y = 0, m_Z = 0;
		size_t m_DrawKey = 0ull;

		std::unique_ptr<BlockCuller> m_Culler;

		std::shared_ptr<GeoThing> m_Geo;
		std::shared_ptr<Material> m_Mat;
		std::shared_ptr<btBoxShape> m_Shape;
		static std::weak_ptr<GeoThing> s_Geo;
		static std::weak_ptr<Material> s_Mats[5];
		static std::weak_ptr<btBoxShape> s_Shape;

		std::shared_ptr<btCollisionObject> m_Body;

		constexpr static Material s_MaterialDatas[5] = 
		{
			Material({ 0.05f, 0.05f, 0.05f, 1.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.8f, 0.8f, 0.8f, 1.f }, { 0.3f, 0.3f, 0.3f, 25.f }, 1, 0),
			Material({ 0.05f, 0.05f, 0.05f, 1.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.5f, 0.5f, 0.5f, 1.f }, { 0.25f, 0.25f, 0.25f, 25.f }, 1, 0),
			Material({ 0.05f, 0.05f, 0.05f, 1.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.8f, 0.5f, 0.5f, 1.f }, { 0.2f, 0.35f, 0.35f, 25.f }, 1, 0), 
			Material({ 0.05f, 0.05f, 0.05f, 1.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.5f, 0.8f, 0.5f, 1.f }, { 0.35f, 0.2f, 0.35f, 25.f }, 1, 0),
			Material({ 0.05f, 0.05f, 0.05f, 1.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.5f, 0.5f, 0.8f, 1.f }, { 0.3f, 0.3f, 0.3f, 25.f }, 1, 0) 
		};
	};*/
}