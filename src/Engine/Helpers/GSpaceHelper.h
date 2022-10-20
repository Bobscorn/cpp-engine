#pragma once

#include "Drawing/Graphics1.h"

#include "Systems/Input/Assimp.h"

#include "Structure/BasicShapes.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Damn
{
	struct SceneAttachThings
	{

	};

	// Be careful with this, leaves loose references to geometry and materials (simply attach even a single draw call using these and they'll be tight again)
	struct RawNode
	{
		std::string Name;
		std::shared_ptr<struct Material> Material;
		std::shared_ptr<GeoThing> Geometry;
		Matrixy4x4 World;
	};

	struct Scene
	{
		struct FailedImport : std::exception
		{
			virtual const char *what() const noexcept
			{
				return "Failed to import scene";
			}
		};

		struct FailedToGetPull : std::exception
		{
			virtual const char *what() const noexcept
			{
				return "Failed to get pulled scene";
			}
		};

		Scene(std::string filename, PullOptions opts = PullOptions{});
		Scene(const aiScene *scene, PullOptions opts = PullOptions{});
		Scene(const PulledScene &scene);

		bool TestForAIScene(); // Attemps to import Assimp scene (if not already), *then* returns whether or not it has the Assimp scene
		bool TestForPulledScene() noexcept; // Attempts to pull a scene if none has been, *then* returns whether or not there is a scene

		std::string GetFileName() const;
		const aiScene *GetAIScene();
		const PulledScene &GetPulledScene();

	protected:
		std::string m_FileName;
		Assimp::Importer m_AIImp;
		const aiScene *m_AIScene = nullptr;
		PullOptions m_PullOptions;
		PulledScene m_PulledScene;
		bool m_HasBaseInfo = false; // Base info means filename and the Assimp Importer
		bool m_BeenPulled = false;
	};

	bool AttachFileToShape(G1::IShape *shape, std::string name, IRen3D *ren);
	bool AttachSceneToShape(G1::IShape *shape, const aiScene *scene, IRen3D *ren);
	bool AttachSceneToShape(G1::IShape *shape, const PulledScene &scene, IRen3D *ren);

	// Adds geometry and materials to the IRen3D pointer
	// Returns a node containing the keys
	bool GetRawNodeFromScene(const PulledScene &scene, std::string nodename, RawNode &out, IRen3D *ren);

	bool AddNodeAsStaticBody(std::string name, PulledScene &scene, G1::IShape *parent, std::string shapename);
	inline bool AddNodeAsStaticBody(std::string name, PulledScene &scene, G1::IShape *parent)
	{
		return AddNodeAsStaticBody(name, scene, parent, name);
	}

	struct SubImporter
	{
		virtual ~SubImporter() {}

		virtual bool ImportNode(const PulledNode &node, const PulledScene &scene, G1::IShape *target) noexcept = 0; // Returning true means don't turn node into a renderable shape

		inline virtual void Finalize(const PulledScene& scene, G1::IShape* target) noexcept { (void)scene; (void)target; };
	};

	struct ImportOptions
	{
		std::vector<std::shared_ptr<SubImporter>> Importers;
	};

	void ImportSceneToShape(ImportOptions options, Scene &scene, G1::IShape *target);

	struct Level1PhysicsImporter : SubImporter
	{
		virtual bool ImportNode(const PulledNode &node, const PulledScene &scene, G1::IShape *target) noexcept override;

	protected:
		static void ImportAsCube(const PulledNode &node, const PulledScene &scene, G1::IShape *target) noexcept;
		static void ImportAsBvh(const PulledNode &node, const PulledScene &scene, G1::IShape *target) noexcept;
	};

	struct Level1PerviousImporter : SubImporter
	{
		virtual bool ImportNode(const PulledNode &node, const PulledScene &scene, G1::IShape *target) noexcept override;

		virtual void Finalize(const PulledScene &scene, G1::IShape *target) noexcept override;

	protected:

		struct SingleButton
		{
			Matrixy4x4 ButtonTransform;
			Matrixy4x4 WallTransform;
			btVector3 Normal;
			btVector3 ButtonNormal;
			bool Pre_Activated = false;
		};

		struct ButtonData
		{
			Matrixy4x4 Transform;
			btVector3 Normal;
		};

		struct WallData
		{
			Matrixy4x4 Transform;
			btVector3 Normal;
			bool Pre_Activated = false;
		};

		struct PadData
		{
			Matrixy4x4 Transform;
		};

		struct GroupData
		{
			std::unordered_map<size_t, WallData> Walls;
			size_t Count = 0ull;
			bool Inverted = false;
		};

		struct PairData
		{
			size_t First = (size_t)-1;
			size_t Second = (size_t)-1;
			bool Copy = false;
		};

		std::unordered_map<size_t, ButtonData> Buttons;
		std::unordered_map<size_t, WallData> Walls;
		std::unordered_map<size_t, PadData> Pads;
		std::unordered_map<size_t, GroupData> Groups;
		std::unordered_map<size_t, PairData> Pairs;
		std::vector<PadData> EndPads;
		std::vector<ButtonData> EndButtons;

		void ImportButtonEnd(const PulledNode &buttonnode, const PulledScene &scene, G1::IShape *target) noexcept;
		void ImportPadEnd(const PulledNode &buttonnode, const PulledScene &scene, G1::IShape *target) noexcept;
		void ImportButton(const PulledNode &buttonnode, size_t number, const PulledScene &scene, G1::IShape *target) noexcept;
		void ImportWall(const PulledNode &wall, size_t number, bool activated, const PulledScene &scene, G1::IShape *target) noexcept;
		void ImportPad(const PulledNode &wall, size_t number, const PulledScene &scene, G1::IShape *target) noexcept;
		void ImportPair(const PulledNode &wall, size_t wallid, size_t pairid);

		void ImportGroupControl(size_t group_id, size_t platcount, bool invert);
		void ImportGroupWall(const PulledNode &wall, size_t group_id, size_t number, bool activated, const PulledScene &scene, G1::IShape *target) noexcept;

		void ImportPlayer(const PulledNode& node, const PulledScene &scene, G1::IShape *target);
	};

	inline std::vector<std::shared_ptr<SubImporter>> GetLevel1ImportOptions()
	{
		return { std::make_shared<Level1PhysicsImporter>(), std::make_shared<Level1PerviousImporter>() };
	}
}

namespace DagNabbit
{
	// A bunch of singleton objects aimed at optimizing and easing some more common uses of physics shapes and geometry
	class DefaultObjects
	{
	public:

		static std::shared_ptr<btBoxShape> GetDefaultCube();
		static std::shared_ptr<btBoxShape> GetDoubleCube();
		static std::shared_ptr<GeoThing> GetDefaultCubeGeo(IRen3D *ren);
		static std::shared_ptr<struct Material> GetDefaultMaterial();

	protected:
		static std::shared_ptr<btBoxShape> DefaultCube; // 0.5f, 0.5f, 0.5f
		static std::shared_ptr<btBoxShape> DoubleCube; // 1.f, 1.f, 1.f

		static std::weak_ptr<GeoThing> DefaultCubeGeoKey;
		static std::shared_ptr<struct Material> DefaultMaterial;

		constexpr static FullVertex CubeVertices[24] = {
			FullVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-1.000000f, +0.000001f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-0.000001f, -1.000000f, +0.000000f), floaty3(+1.000000f, -0.000001f, -0.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, -0.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+1.000000f, +0.000000f) },
		};
		constexpr static std::array<unsigned int, 36> CubeIndices = { 2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9, 14, 13, 12, 17, 16, 15, 1, 18, 0, 4, 19, 3, 7, 20, 6, 10, 21, 9, 13, 22, 12, 16, 23, 15, };

		constexpr static Material DefaultCubeMaterial = Material( floaty4{0.f, 0.f, 0.f, 0.f}, floaty4{0.f, 0.f, 0.f, 0.f}, floaty4{ 0.8f, .026f, .781f, 1.f}, floaty4{ 1.f, 1.f, 1.f, 3.f }, 1.f, 0u );
	};
}