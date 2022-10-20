#include "GSpaceHelper.h"

#include "StringHelper.h"
#include "MathHelper.h"

#include "Structure/PerviousShapes.h"

#include <regex>

std::shared_ptr<btBoxShape> DagNabbit::DefaultObjects::DefaultCube = nullptr; // 0.5f, 0.5f, 0.5f
std::shared_ptr<btBoxShape> DagNabbit::DefaultObjects::DoubleCube = nullptr; // 1.f, 1.f, 1.f

std::weak_ptr<GeoThing> DagNabbit::DefaultObjects::DefaultCubeGeoKey{};
std::shared_ptr<struct Material> DagNabbit::DefaultObjects::DefaultMaterial = nullptr;

bool Damn::AttachFileToShape(G1::IShape * shape, std::string name, IRen3D *ren)
{
	Assimp::Importer imp;
	
	return AttachSceneToShape(shape, imp.ReadFile(name, aiProcess_ValidateDataStructure |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices), ren);
}

bool Damn::AttachSceneToShape(G1::IShape * shape, const aiScene * scene, IRen3D *ren)
{
	if (!shape)
	{
		DERROR("Cannot attach scene to null shape");
		return false;
	}
	if (!scene)
	{
		DERROR("Cannot attach null scene to shape");
		return false;
	}

	PulledScene pulledscene = PullScene(scene);

	return AttachSceneToShape(shape, pulledscene, ren);
}

bool Damn::AttachSceneToShape(G1::IShape * shape, const PulledScene & scene, IRen3D * ren)
{
	if (scene.Nodes.empty())
	{
		DWARNING("Pulled Scene was empty");
		return false;
	}

	G1::IShapeThings ting{ shape->GetContainer(), shape->GetResources(), "Unnamed Shape" };

	std::unordered_map<GeoMatCombo, std::shared_ptr<GeoThing>> GeoNameToKey;
	GeoNameToKey.reserve(scene.Geometries.size());
	for (auto &geo : scene.Geometries)
	{
		std::shared_ptr<GeoThing> g = ren->AddGeometry(geo.second.Vertices, geo.second.Indices);
		GeoNameToKey[geo.first] = g;
	}

	std::unordered_map<std::string, std::shared_ptr<struct Material>> MatNameToPtr;
	MatNameToPtr.reserve(scene.Materials.size());
	for (auto &mat : scene.Materials)
	{
		MatNameToPtr[mat.first] = std::make_shared<struct Material>(mat.second);
	}

	for (auto &node : scene.Nodes)
	{
		Drawing::DrawCall call;
		auto g_it = GeoNameToKey.find({ node.GeoName, node.MaterialName });
		if (g_it != GeoNameToKey.end())
		{
			call.Geometry = g_it->second;
		}
		else
		{
			DWARNING("Node '" + node.NodeName + "' in Pulled Scene '" + scene.Name + "' has invalid Geometry Name (being skipped)");
			continue;
		}
		auto m_it = MatNameToPtr.find(node.MaterialName);
		if (m_it != MatNameToPtr.end())
		{
			call.Material = m_it->second;
		}
		else
		{
			DWARNING("Node '" + node.NodeName + "' in Pulled Scene '" + scene.Name + "' has invalid Material Name (being skipped)");
			continue;
		}
		ting.Name = node.NodeName;
		shape->AddChild(new G1I::RawMatrixRendyShape(ting, call, node.FlattenedTransform));
	}

	return true;
}

bool Damn::GetRawNodeFromScene(const PulledScene & scene, std::string nodename, RawNode & out, IRen3D *ren)
{
	if (!ren)
	{
		DERROR("Cannot create RawNode with a null IRen3D pointer");
		return false;
	}
	if (scene.Nodes.empty())
	{
		DERROR("Cannot retrieve any nodes from an empty scene");
		return false;
	}

	auto it = scene.Nodes.find({ nodename });
	if (it == scene.Nodes.end())
	{
		DERROR("Could not find node in scene");
		return false;
	}
	auto geoit = scene.Geometries.find({ it->GeoName, it->MaterialName });
	if (geoit == scene.Geometries.end())
	{
		DERROR("Could not find node's geometry");
		return false;
	}
	auto matit = scene.Materials.find(it->MaterialName);
	if (matit == scene.Materials.end())
	{
		DERROR("Could not find node's material (unusual considering its required to find the geometry, which was found");
		return false;
	}
	out.Geometry = ren->AddGeometry(geoit->second.Vertices, geoit->second.Indices);
	out.Material = std::make_shared<struct Material>(matit->second);
	out.Name = it->NodeName;
	out.World = it->FlattenedTransform;

	return true;
}

bool Damn::AddNodeAsStaticBody(std::string name, PulledScene & scene, G1::IShape * parent, std::string shapename)
{
	G1::IShapeThings ting;
	ting.Container = parent->GetContainer();
	ting.Resources = parent->GetResources();
	G1I::PhysicsThing ting2;
	ting2.Mass = 0.f;
	ting2.Dynamic = false;
	auto nodesearch = scene.Nodes.find({ name });
	if (nodesearch != scene.Nodes.end())
	{
		auto geosearch = scene.Geometries.find({ nodesearch->GeoName, nodesearch->MaterialName });
		if (geosearch != scene.Geometries.end())
		{
			ting2.Trans = nodesearch->FlattenedTransform;
			G1I::BvhThings ting3; 
			ting3.Vertices.resize(geosearch->second.Vertices.size());
			ting3.Indices.resize(geosearch->second.Indices.size());

			for (size_t i = geosearch->second.Vertices.size(); i-- > 0; )
				ting3.Vertices[i] = geosearch->second.Vertices[i].PosL;
			std::copy(geosearch->second.Indices.begin(), geosearch->second.Indices.end(), ting3.Indices.begin());

			/*DINFO("Debug output for triangles");
			DINFO("Outputting Vertices:");
			for (auto& vertex : ting3.Vertices)
			{
				DINFO(AsString(vertex));
			}
			DINFO("------------Indices-----------");
			for (auto& indices : ting3.Indices)
			{
				DINFO(std::to_string(indices));
			}
			DINFO("--------------Triangles----------");
			for (size_t i = 0; i < ting3.Indices.size(); i += 3)
			{
				DINFO(StringHelper::NumToPlace(i / 3) + " Tri: \t" + AsString(ting3.Vertices[ting3.Indices[i + 0]]));
				DINFO("\t\t\t\t" + AsString(ting3.Vertices[ting3.Indices[i + 1]]));
				DINFO("\t\t\t\t" + AsString(ting3.Vertices[ting3.Indices[i + 2]]));
			}
			DINFO("-----------Debug Check------------");
			bool found = false;
			for (size_t i = 0; i < ting3.Indices.size(); i += 3)
			{
				auto v1 = ting3.Vertices[ting3.Indices[i]];
				auto v2 = ting3.Vertices[ting3.Indices[i + 1]];
				auto v3 = ting3.Vertices[ting3.Indices[i + 2]];

				if (v1 == v2 || v1 == v3 || v2 == v3)
				{
					found = true;
					DINFO("Triangle " + StringHelper::NumToPlace(i / 3) + " is bad");
				}
			}
			if (!found)
				DINFO("No bad triangles found");*/

			parent->AddChild(new G1I::BvhPhysicsBody(ting, ting2, ting3));
			return true;
		}
	}
	return false;
}

Damn::Scene::Scene(std::string filename, PullOptions opts) : m_FileName(filename), m_PullOptions(opts), m_HasBaseInfo(true)
{
}

Damn::Scene::Scene(const aiScene * scene, PullOptions opts) : m_AIScene(scene), m_PullOptions(opts), m_HasBaseInfo(false)
{
}

Damn::Scene::Scene(const PulledScene & scene) : m_PulledScene(scene)
{
}

bool Damn::Scene::TestForAIScene()
{
	if (m_AIScene)
		return m_AIScene;

	if (m_HasBaseInfo)
	{
		if (m_FileName.size())
		{
			m_AIScene = m_AIImp.ReadFile(m_FileName.c_str(), aiProcess_ValidateDataStructure |
				aiProcess_GenSmoothNormals |
				aiProcess_CalcTangentSpace |
				aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices);

			return m_AIScene;
		}
	}

	return false;
}

bool Damn::Scene::TestForPulledScene() noexcept
{
	if (m_BeenPulled)
		return true;

	if (m_AIScene)
	{
		m_PulledScene = PullScene(m_AIScene, m_PullOptions);

		m_BeenPulled = true;

		return m_PulledScene.Nodes.size();
	}

	if (m_HasBaseInfo)
	{
		try
		{
			(void)GetAIScene();
		}
		catch (const FailedImport & e)
		{
			(void)e;
			return false;
		}
		if (!m_AIScene)
			return false;

		m_PulledScene = PullScene(m_AIScene, m_PullOptions);

		m_BeenPulled = true;

		return m_PulledScene.Nodes.size();
	}

	return false;
}

std::string Damn::Scene::GetFileName() const
{
	return m_FileName;
}

const aiScene * Damn::Scene::GetAIScene()
{
	if (m_AIScene)
		return m_AIScene;

	if (m_HasBaseInfo)
	{
		if (m_FileName.size())
		{
			m_AIScene = m_AIImp.ReadFile(m_FileName.c_str(), aiProcess_ValidateDataStructure |
				aiProcess_GenSmoothNormals |
				aiProcess_CalcTangentSpace |
				aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices);

			if (!m_AIScene)
				throw FailedImport();

			return m_AIScene;
		}
	}

	throw FailedImport();
}

const PulledScene & Damn::Scene::GetPulledScene()
{
	if (m_BeenPulled)
	{
		return m_PulledScene;
	}

	if (m_AIScene)
	{
		m_PulledScene = PullScene(m_AIScene, m_PullOptions);

		m_BeenPulled = true;

		return m_PulledScene;
	}

	if (m_HasBaseInfo)
	{
		(void)GetAIScene();

		if (!m_AIScene)
			throw FailedImport();

		m_PulledScene = PullScene(m_AIScene, m_PullOptions);

		m_BeenPulled = true;
		
		return m_PulledScene;
	}

	throw FailedToGetPull();
}




void Damn::ImportSceneToShape(ImportOptions options, Scene & scene, G1::IShape * target)
{
	if (scene.TestForPulledScene())
	{
		auto &ren3 = target->GetResources()->Ren3;
		auto &p = scene.GetPulledScene();

		std::unordered_map<GeoMatCombo, std::shared_ptr<GeoThing>> GeoNameToKey;
		std::unordered_map<std::string, std::shared_ptr<Material>> MatNameToPtr;
		G1::IShapeThings ting;
		ting.Container = target->GetContainer();
		ting.Resources = target->GetResources();

		for (auto & node : p.Nodes)
		{
			bool Imported = false;
			for (auto &importer : options.Importers)
			{
				if (importer->ImportNode(node, p, target))
				{
					Imported = true;
					break;
				}
			}
			if (Imported)
				continue;

			ting.Name = node.NodeName;

			Drawing::DrawCall call;

			auto existit = GeoNameToKey.find({ node.GeoName, node.MaterialName });
			if (existit == GeoNameToKey.end())
			{
				// Default import behaviour
				auto geoit = p.Geometries.find(GeoMatCombo{ node.GeoName, node.MaterialName });
				if (geoit == p.Geometries.end())
					continue;
				auto matit = p.Materials.find(node.MaterialName);
				if (matit == p.Materials.end())
					continue;

				std::shared_ptr<GeoThing> geoguccy = ren3->AddGeometry(geoit->second.Vertices, geoit->second.Indices);
				GeoNameToKey[GeoMatCombo{ node.GeoName, node.MaterialName }] = geoguccy;

				call.Geometry = geoguccy;
			}
			else
				call.Geometry = existit->second;

			auto matexistit = MatNameToPtr.find(node.MaterialName);
			if (matexistit == MatNameToPtr.end())
			{
				auto matit = p.Materials.find(node.MaterialName);
				if (matit == p.Materials.end())
					continue;

				auto mat = std::make_shared<Material>(matit->second);
				MatNameToPtr[node.MaterialName] = mat;

				call.Material = mat;
			}
			else
				call.Material = matexistit->second;

			target->AddChild(new G1I::RawMatrixRendyShape(ting, call, node.FlattenedTransform));
		}

		for (auto &imp : options.Importers)
			imp->Finalize(p, target);


		for (auto &light : p.Lights)
		{
			target->AddChild(new G1I::LightShape(ting.WithName("Light " + light.first), light.second));
		}
	}

}

bool Damn::Level1PhysicsImporter::ImportNode(const PulledNode & node, const PulledScene & scene, G1::IShape * target) noexcept
{
	const std::string &name = node.NodeName;

	static const char *RenderRegex = "Render";
	static const char *CubeRegex = "^Cube";
	static const char *BvhRegex = "^Bvh";
	std::regex RenderReg{ RenderRegex };

	bool matched = std::regex_search(name, RenderReg);

	bool cube = std::regex_search(name, std::regex{ CubeRegex });

	if (cube)
	{
		ImportAsCube(node, scene, target);

		if (matched)
			return false;
		return true;
	}

	bool bvh = std::regex_search(name, std::regex{ BvhRegex });

	if (bvh)
	{
		ImportAsBvh(node, scene, target);

		if (matched)
			return false; // Returning false tells the Import function to add this node as a render function (assuming no other importers return true)
		return true;
	}

	return false;
}

void Damn::Level1PhysicsImporter::ImportAsCube(const PulledNode & node, const PulledScene & scene, G1::IShape * target) noexcept
{
	G1::IShapeThings ting;
	ting.Name = node.NodeName;
	ting.Resources = target->GetResources();
	ting.Container = target->GetContainer();
	
	auto it = scene.Geometries.find({ node.GeoName, node.MaterialName });

	if (it == scene.Geometries.end())
	{
		DWARNING("Can not import an empty mesh as a cube, requires vertices to determine extents");
		return;
	}

	const std::vector<FullVertex> &vertices = it->second.Vertices;
	const std::vector<unsigned int> &indices = it->second.Indices;

	if (vertices.size() != 8 && (vertices.size() != 24 && indices.size() != 36))
	{
		DWARNING("Can not import a non-cuboid as a cube (yet)");
		return;
	}

	// Determine the 3 'axis' for the box
	std::array<btVector3, 3u> Axes;
	
	
	// Find the axis closest to the z-axis

	btVector3 firsthypotenuse;
	btVector3 & nonhypotenuse1 = Axes[0], & nonhypotenuse2 = Axes[1], & nonhypotenuse3 = Axes[2];
	{
		btVector3 v0 = vertices[indices[0]].PosL;
		btVector3 v1 = vertices[indices[1]].PosL;
		btVector3 v2 = vertices[indices[2]].PosL;

		// Determine the non-hypotenuses

		btVector3 v01 = v0 - v1;
		btVector3 v12 = v1 - v2;
		btVector3 v20 = v2 - v0;

		if (v01.length2() > v12.length2())
		{
			nonhypotenuse1 = v12;
			if (v20.length2() > v01.length2())
			{
				nonhypotenuse2 = v01;
				firsthypotenuse = v20;
			}
			else
			{
				nonhypotenuse2 = v20;
				firsthypotenuse = v01;
			}
		}
		else
		{
			nonhypotenuse1 = v01;
			if (v20.length2() > v12.length2())
			{
				nonhypotenuse2 = v12;
				firsthypotenuse = v20;
			}
			else
			{
				nonhypotenuse2 = v20;
				firsthypotenuse = v12;
			}
		}
	}

	for (size_t i = 12; i-- > 1; )
	{
		btVector3 v0 = vertices[indices[i * 3 + 0]].PosL;
		btVector3 v1 = vertices[indices[i * 3 + 1]].PosL;
		btVector3 v2 = vertices[indices[i * 3 + 2]].PosL;

		// Determine the non-hypotenuses

		btVector3 v01 = v0 - v1;
		btVector3 v12 = v1 - v2;
		btVector3 v20 = v2 - v0;

		if (v01 == firsthypotenuse || v12 == firsthypotenuse || v20 == firsthypotenuse)
			continue;

		if (v01.absolute() == nonhypotenuse1.absolute() || v01.absolute() == nonhypotenuse2.absolute())
		{
			nonhypotenuse3 = ( v12.length2() > v20.length2() ? v20 : v12 );
		}
		else if (v12.absolute() == nonhypotenuse1.absolute() || v12.absolute() == nonhypotenuse2.absolute())
		{
			nonhypotenuse3 = (v01.length2() > v20.length2() ? v20 : v01);
		}
		else
		{
			nonhypotenuse3 = (v01.length2() > v12.length2() ? v12 : v01);
		}
		break;
	}

	// Found the 3 axises




	for (size_t i = 3; i-- > 0; )
		Axes[i].safeNormalize();

	Matrixy4x4 Rotation;
	btVector3 axis;
	if (fabsf(Axes[0].z()) > fabsf(Axes[1].z()))
	{
		if (fabsf(Axes[0].z()) > fabsf(Axes[2].z()))
		{
			// Use Axis 0
			axis = Axes[0];
		}
		else
		{
			// Use Axis 2
			axis = Axes[2];
		}
	}
	else
	{
		if (fabsf(Axes[2].z()) > fabsf(Axes[1].z()))
		{
			// Use Axis 2
			axis = Axes[2];
		}
		else
		{
			// Use Axis 1
			axis = Axes[1];
		}
	}


	float fac = acosf(axis.dot(btVector3{ 0.f, 0.f, -1.f }));

	btVector3 rotaxis = btVector3{ 0.f, 0.f, -1.f }.cross(axis);

	Rotation = Matrixy4x4::RotationAxisR((floaty3)rotaxis, fac);

	Matrixy4x4 Inverted;
	if (!Matrixy4x4::Inverted(Rotation, Inverted))
	{
		DERROR("Somehow failed to invert the rotation matrix, can not continue");
		return;
	}

	// For now we're assuming the origin is in the center because fml if I'm going to work out a dynamic center

	floaty3 e = Inverted.TransformNormal(vertices[0].PosL);
	btVector3 extents = { fabsf(e.x), fabsf(e.y), fabsf(e.z) };

	G1I::PhysicsThing ting2;
	ting2.Dynamic = false;
	ting2.Shape = std::make_shared<btBoxShape>(extents);
	ting2.Trans = Matrixy4x4::MultiplyE(node.FlattenedTransform, Rotation);
	ting2.CollisionGroup = ENVIRONMENT;
	ting2.CollisionMask = PLAYER | DYNAMIC_PERVIOUS | DYNAMIC_PERVIOUS_INVERTED | INTERACTION_RAY;

	target->AddChild(new G1I::PhysicsBody(ting, ting2));
}

void Damn::Level1PhysicsImporter::ImportAsBvh(const PulledNode & node, const PulledScene &scene, G1::IShape * target) noexcept
{
	G1::IShapeThings ting = { target->GetContainer(), target->GetResources(), node.NodeName };
	G1I::PhysicsThing ting2;
	ting2.Dynamic = false;
	ting2.CollisionGroup = ENVIRONMENT;
	ting2.CollisionMask = PLAYER | DYNAMIC_PERVIOUS | DYNAMIC_PERVIOUS_INVERTED | INTERACTION_RAY;

	auto geosearch = scene.Geometries.find({ node.GeoName, node.MaterialName });
	if (geosearch != scene.Geometries.end())
	{
		ting2.Trans = node.FlattenedTransform;
		G1I::BvhThings ting3;
		ting3.Vertices.resize(geosearch->second.Vertices.size());
		ting3.Indices.resize(geosearch->second.Indices.size());

		for (size_t i = geosearch->second.Vertices.size(); i-- > 0; )
			ting3.Vertices[i] = geosearch->second.Vertices[i].PosL;
		std::copy(geosearch->second.Indices.begin(), geosearch->second.Indices.end(), ting3.Indices.begin());
		
		target->AddChild(new G1I::BvhPhysicsBody(ting, ting2, ting3));
		return;
	}
	else
	{
		DWARNING("Could not find geometry for node: " + node.NodeName);
	}
}

bool Damn::Level1PerviousImporter::ImportNode(const PulledNode & node, const PulledScene & scene, G1::IShape * target) noexcept
{
	static const char *ButtonRegex = "^Button(\\d+)";
	static const char *WallRegex = "^(Wall|Platform)(\\d+)";
	static const char *PadRegex = "^Pad(\\d+)";
	static const char *DynamicCubeRegex = "^Dynamic";
	static const char *ActivatedWallRegex = "Activated";
	static const char *EndRegex = "(Button|Pad)(.*?)End";
	static const char *GroupPartRegex = "^(Wall|Platform)(\\d+)Group(\\d+)";
	static const char *GroupRegex = "^Group(\\d+)Plat(\\d+)(Yes|No)";
	static const char *PlayerRegex = "^Player";
	static const char *PairRegex = "Pair(\\d+)";

	const std::string &name = node.NodeName;

	std::smatch match;
	bool is_end = std::regex_search(name, match, std::regex{ EndRegex });

	if (is_end)
	{
		if (match[1] == "Button")
			ImportButtonEnd(node, scene, target);
		else
			ImportPadEnd(node, scene, target);
		
		return true;
	}

	bool isbutton = std::regex_search(name, match, std::regex{ ButtonRegex });

	if (isbutton)
	{
		ImportButton(node, std::stoull(match[1].str()), scene, target);

		return true;
	}

	bool iswall = std::regex_search(name, match, std::regex{ WallRegex });

	if (iswall)
	{
		bool normal_wall = true;
		std::smatch wallmatch;
		bool is_group_wall = std::regex_search(name, wallmatch, std::regex{ GroupPartRegex });
		if (is_group_wall)
		{
			ImportGroupWall(node, std::stoull(wallmatch[3].str()), std::stoull(wallmatch[2].str()), std::regex_search(name, std::regex{ ActivatedWallRegex }), scene, target);
			normal_wall = false;
		}

		if (normal_wall && std::regex_search(name, wallmatch, std::regex{ PairRegex }))
		{
			ImportPair(node, std::stoull(match[2].str()), std::stoull(wallmatch[1].str()));
		}

		if (normal_wall)
			ImportWall(node, std::stoull(match[2].str()), std::regex_search(name, std::regex{ ActivatedWallRegex }), scene, target);

		return true;
	}

	bool is_pad = std::regex_search(name, match, std::regex{ PadRegex });

	if (is_pad)
	{
		ImportPad(node, std::stoull(match[1].str()), scene, target);
		
		return true;
	}

	bool is_dynamic_cube = std::regex_search(name, match, std::regex{ DynamicCubeRegex });

	if (is_dynamic_cube)
	{
		target->AddChild(new Perviousity::Shapes::CubeyBoi({ target->GetContainer(), target->GetResources(), match.suffix() }, { node.FlattenedTransform }));

		return true;
	}

	bool is_group_control = std::regex_search(name, match, std::regex{ GroupRegex });

	if (is_group_control)
	{
		ImportGroupControl(std::stoull(match[1].str()), std::stoull(match[2].str()), (match[3].str() == "Yes" ? true : false));

		return true;
	}

	if (std::regex_search(name, match, std::regex{ PlayerRegex }))
	{
		ImportPlayer(node, scene, target);

		return true;
	}

	return false;
}

void Damn::Level1PerviousImporter::Finalize(const PulledScene & scene, G1::IShape * target) noexcept
{
	G1::IShapeThings ting = { target->GetContainer(), target->GetResources(), "Unnamed Wall" };

	// Import the Activatables (walls) first

	std::unordered_map<size_t, Pointer::f_ptr<Perviousity::Activators::IActivatable>> CreatedWalls;

	for (auto &e : Pairs)
	{
		auto it1 = Walls.find(e.second.First);
		auto it2 = Walls.find(e.second.Second);
		if (it1 == Walls.end() || it2 == Walls.end())
		{
			continue;
		}
		auto &wall1 = it1->second;
		auto &wall2 = it2->second;
		floaty3 translate1 = wall1.Transform.Transform({ 0.f, 0.f, 0.f }), translate2 = wall2.Transform.Transform({ 0.f, 0.f, 0.f });
		Pointer::f_ptr<Perviousity::Shapes::PairedPerviousObject> pair1 = target->AddChild(new Perviousity::Shapes::PairedPerviousObject(ting.WithName("Pair " + std::to_string(e.first) + "'s first"), { Matrixy4x4::Translate(translate1), wall1.Normal, nullptr, true, e.second.Copy }, wall1.Pre_Activated)).SketchyCopy<Perviousity::Shapes::PairedPerviousObject>();
		Pointer::f_ptr<Perviousity::Shapes::PairedPerviousObject> pair2 = target->AddChild(new Perviousity::Shapes::PairedPerviousObject(ting.WithName("Pair " + std::to_string(e.first) + "'s second"), { Matrixy4x4::Translate(translate2), wall2.Normal, pair1, true, e.second.Copy }, wall2.Pre_Activated)).SketchyCopy<Perviousity::Shapes::PairedPerviousObject>();
		pair1->SetPair(pair2);
		Walls.erase(e.second.First);
		Walls.erase(e.second.Second);
		CreatedWalls[e.second.First] = pair1.SketchyCopy<Perviousity::Activators::IActivatable>();
		CreatedWalls[e.second.Second] = pair2.SketchyCopy<Perviousity::Activators::IActivatable>();
	}

	for (auto &e : Walls)
	{
		if (e.second.Transform.IsBlank())
			continue;

		floaty3 translate = e.second.Transform.Transform({ 0.f, 0.f, 0.f });
		CreatedWalls[e.first] = target->AddChild(new Perviousity::Shapes::ActivateOnlyWall(ting.WithName("PerviousWall No. " + std::to_string(e.first)), { Matrixy4x4::Translate(translate), e.second.Normal, e.second.Pre_Activated })).SketchyCopy<Perviousity::Activators::IActivatable>();
	}


	// Now import the activators

	for (auto &e : Buttons)
	{
		auto wallit = CreatedWalls.find(e.first);
		if (wallit == CreatedWalls.end())
			continue;

		if (e.second.Transform.IsBlank())
			continue;

		try
		{
			target->AddChild(new Perviousity::Activators::StandButton(ting.WithName("PerviousButton No. " + std::to_string(e.first)), { wallit->second, e.second.Transform, e.second.Normal }));
		}
		catch (const Perviousity::Activators::StandButton::EnsureFailure &e)
		{
			DERROR("Could not create Stand button: " + e.what());
		}
	}

	for (auto &e : Pads)
	{
		auto wallit = CreatedWalls.find(e.first);
		if (wallit == CreatedWalls.end())
			continue;

		if (e.second.Transform.IsBlank())
			continue;

		try
		{
			target->AddChild(new Perviousity::Activators::PressurePad(ting.WithName("PerviousPad No. " + std::to_string(e.first)), { wallit->second, e.second.Transform }));
		}
		catch (const PullException &e)
		{
			DERROR("Could not create PressurePad: " + e.what());
		}
	}

	Pointer::f_ptr<Perviousity::Activators::IActivatable> End;
	if (EndPads.size() || EndButtons.size())
	{
		End = target->AddChild(new Perviousity::Activators::Ender(ting.WithName("Level Ender"))).SketchyCopy<Perviousity::Activators::IActivatable>();
	}

	for (auto &e : EndPads)
	{
		try
		{
			target->AddChild(new Perviousity::Activators::PressurePad(ting.WithName("Pervious Pad Ender"), { End, e.Transform }));
		}
		catch (const PullException & e)
		{
			DERROR("Could not create Ending Pressure Pad: " + e.what());
		}
	}

	for (auto &e : EndButtons)
	{
		try
		{
			target->AddChild(new Perviousity::Activators::StandButton(ting.WithName("Pervious Button Ender"), { End, e.Transform, e.Normal, true }));
		}
		catch (const PullException & e)
		{
			DERROR("Could not create Ending Stand Button: " + e.what());
		}
	}

	for (auto &group : Groups)
	{
		if (group.second.Count)
		{
			auto control = target->AddChild(new Perviousity::Shapes::GroupController(ting.WithName("Group Control " + std::to_string(group.first)), { group.second.Count, group.second.Inverted })).SketchyCopy<Perviousity::Shapes::GroupController>();
			for (auto &wall : group.second.Walls)
			{
				floaty3 translate = wall.second.Transform.Transform({ 0.f, 0.f, 0.f });
				control->AddChild<Perviousity::Shapes::GroupPerviousObject>(Matrixy4x4::Translate(translate), wall.second.Normal, wall.second.Pre_Activated);
			}
		}
	}
}

void Damn::Level1PerviousImporter::ImportButtonEnd(const PulledNode &buttonnode, const PulledScene &scene, G1::IShape *target) noexcept
{
	(void)target;
	auto geosearch = scene.Geometries.find({ buttonnode.GeoName, buttonnode.MaterialName });

	btVector3 Normal;

	if (geosearch != scene.Geometries.end())
		Normal = buttonnode.FlattenedTransform.TransformNormal(geosearch->second.Vertices[0].NormalL);
	else
		Normal = { 0.f, 1.f, 0.f };

	EndButtons.emplace_back(ButtonData{ buttonnode.FlattenedTransform, Normal });
}

void Damn::Level1PerviousImporter::ImportPadEnd(const PulledNode &buttonnode, const PulledScene &scene, G1::IShape *target) noexcept
{
	(void)scene;
	(void)target;
	EndPads.emplace_back(PadData{ buttonnode.FlattenedTransform });
}

void Damn::Level1PerviousImporter::ImportButton(const PulledNode & buttonnode, size_t number, const PulledScene & scene, G1::IShape * target) noexcept
{
	(void)target;
	auto geosearch = scene.Geometries.find({ buttonnode.GeoName, buttonnode.MaterialName });

	if (geosearch != scene.Geometries.end())
		Buttons[number].Normal = buttonnode.FlattenedTransform.TransformNormal(geosearch->second.Vertices[0].NormalL);
	else
		Buttons[number].Normal = { 0.f, 1.f, 0.f };

	Buttons[number].Transform = buttonnode.FlattenedTransform;
}

void Damn::Level1PerviousImporter::ImportWall(const PulledNode & wall, size_t number, bool activated, const PulledScene & scene, G1::IShape * target) noexcept
{
	(void)target;
	auto geosearch = scene.Geometries.find({ wall.GeoName, wall.MaterialName });

	if (geosearch == scene.Geometries.end())
	{
		DWARNING("Not importing wall because its geometry can not be found");
		return;
	}

	if (geosearch->second.Vertices.size() != 4ull)
	{
		DWARNING("Not importing wall because it doesn't have only 4 vertices");
		return;
	}

	Walls[number].Transform = wall.FlattenedTransform;
	Walls[number].Normal = wall.FlattenedTransform.TransformNormal(geosearch->second.Vertices[0].NormalL);
	//Walls[number].Normal = (wall.FlattenedTransform.TransformNormal(geosearch->second.Vertices[0].TangentL).cross(wall.FlattenedTransform.TransformNormal(geosearch->second.Vertices[0].BinormalL)));
	Walls[number].Pre_Activated = activated;
}

void Damn::Level1PerviousImporter::ImportPad(const PulledNode & wall, size_t number, const PulledScene & scene, G1::IShape * target) noexcept
{
	(void)scene;
	(void)target;
	Pads[number].Transform = wall.FlattenedTransform;
}

void Damn::Level1PerviousImporter::ImportPair(const PulledNode &wall, size_t wallid, size_t pairid)
{
	const char *CopyRegex = "(No)*Copy";

	std::smatch match;
	if (std::regex_search(wall.NodeName, match, std::regex{ CopyRegex }))
	{

	}

	auto it = Pairs.find(pairid);
	if (it != Pairs.end())
	{
		it->second.Second = wallid;
	}
	else
	{
		Pairs[wallid].First = wallid;
	}
}

void Damn::Level1PerviousImporter::ImportGroupControl(size_t group_id, size_t platcount, bool invert)
{
	Groups[group_id].Count = platcount;
	Groups[group_id].Inverted = invert;
}

void Damn::Level1PerviousImporter::ImportGroupWall(const PulledNode &wall, size_t group_id, size_t number, bool activated, const PulledScene &scene, G1::IShape *target) noexcept
{
	auto geosearch = scene.Geometries.find({ wall.GeoName, wall.MaterialName });

	if (geosearch == scene.Geometries.end())
	{
		DWARNING("Not importing group wall due to lack of geometry");
		return;
	}

	if (geosearch->second.Vertices.size() != 4ull)
	{
		DWARNING("Not importing group wall due to incorrent vertex count (not 4)");
		return;
	}

	auto &group = Groups[group_id];
	group.Walls[number].Transform = wall.FlattenedTransform;
	group.Walls[number].Normal = wall.FlattenedTransform.TransformNormal(geosearch->second.Vertices[0].NormalL);
	group.Walls[number].Pre_Activated = activated;
}

void Damn::Level1PerviousImporter::ImportPlayer(const PulledNode& node, const PulledScene& scene, G1::IShape* target)
{
	Perviousity::Player::PlayerStuff stuff;
	// Get the transform first
	stuff.InitialPosition = node.FlattenedTransform.Transform({ 0.f, 0.f, 0.f });

	// Get the geometry
	auto geosearch = scene.Geometries.find({ node.GeoName, node.MaterialName });
	if (geosearch != scene.Geometries.end())
	{
		if (geosearch->second.Vertices.size())
		{
			stuff.Direction = node.FlattenedTransform.TransformNormal(geosearch->second.Vertices[0].NormalL);
		}
	}

	target->AddChild(new Perviousity::Player::PerviousPlayer(target->GetContainer(), target->GetResources(), stuff));
}

std::shared_ptr<btBoxShape> DagNabbit::DefaultObjects::GetDefaultCube()
{
	if (!DefaultCube)
		DefaultCube = std::make_shared<btBoxShape>(btVector3(0.5f, 0.5f, 0.5f));
	return DefaultCube;
}

std::shared_ptr<btBoxShape> DagNabbit::DefaultObjects::GetDoubleCube()
{
	if (!DoubleCube)
		DoubleCube = std::make_shared<btBoxShape>(btVector3(1.f, 1.f, 1.f));
	return DoubleCube;
}

std::shared_ptr<GeoThing> DagNabbit::DefaultObjects::GetDefaultCubeGeo(IRen3D *ren)
{
	auto locked = DefaultCubeGeoKey.lock();
	if (locked)
		return locked;

	locked = ren->AddGeometry(std::vector<FullVertex>(CubeVertices, CubeVertices + 24), std::vector<unsigned int>(CubeIndices.cbegin(), CubeIndices.cend()));
	DefaultCubeGeoKey = locked;
	return locked;
}

std::shared_ptr<struct Material> DagNabbit::DefaultObjects::GetDefaultMaterial()
{
	if (!DefaultMaterial)
		DefaultMaterial = std::make_shared<struct Material>(DefaultCubeMaterial);
	return DefaultMaterial;
	
}
