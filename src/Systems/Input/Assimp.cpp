#include "Assimp.h"

#include "Helpers/DebugHelper.h"
#include "Helpers/MathHelper.h"

#include <assimp/mesh.h>
#include <assimp/material.h>

#include <assert.h>

std::unordered_map<aiLightSourceType, unsigned int> LightMap = { {aiLightSource_POINT, LIGHT_POINT}, {aiLightSource_SPOT, LIGHT_SPOT}, {aiLightSource_DIRECTIONAL, LIGHT_DIRECTION} };

//void DoThing()
//{
//	std::vector<std::string> files = FileHelper::GetDirectoryFiles(FileHelper::WorkingDirectory());
//	std::vector<std::string> relevantfiles;
//	relevantfiles.resize(files.size());
//	auto last = std::copy_if(files.begin(), files.end(), relevantfiles.begin(), [](std::string in) -> bool { if (in.size() > 4) if (in.substr(in.size() - 4, 4) == ".dae") return true; return false; });
//	relevantfiles.resize(std::distance(relevantfiles.begin(), last));
//
//	if (relevantfiles.empty())
//	{
//		std::cout << "No Files Here" << std::endl;
//		return 0;
//	}
//
//	std::stringstream filedialogue;
//	{
//		auto &f = filedialogue;
//		f << "Files are: " << std::endl;
//		size_t i = 0;
//		for (const auto & file : relevantfiles)
//		{
//			f << "[" << i << "] - " << file << std::endl;
//			++i;
//		}
//	}
//
//	int inindex = -2;
//	while (true)
//	{
//		std::cout << filedialogue.str();
//		inindex = -2;
//		std::cout << "Enter Index: (-1 quit)";
//		std::cin >> inindex;
//		if (inindex == -1)
//			return 0;
//		if (inindex >= 0 && inindex < relevantfiles.size())
//			break;
//	}
//
//	Assimp::Importer imp{};
//	auto * scene = imp.ReadFile(relevantfiles[inindex],
//		aiProcess_ValidateDataStructure |
//		aiProcess_GenSmoothNormals |
//		aiProcess_CalcTangentSpace |
//		aiProcess_Triangulate |
//		aiProcess_JoinIdenticalVertices |
//		aiProcess_SortByPType);
//
//	constexpr auto &tcs = StringHelper::ToCPPString;
//
//	if (scene)
//	{
//		if (scene->HasMeshes())
//		{
//			int index = 0;
//			aiMesh * targetmesh = nullptr;
//			while (true)
//			{
//				if (scene->mNumMeshes > 1)
//				{
//					std::cout << "Scene has " << scene->mNumMeshes << " meshes:" << std::endl;
//					for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
//					{
//						std::cout << "[" << i << "][" << scene->mMeshes[i]->mName.C_Str() << "]" << std::endl;
//					}
//					std::string in;
//					std::cin >> in;
//					if (!StringHelper::IfINT(in, &index))
//					{
//						bool asdfafg = false;
//						for (unsigned int i = scene->mNumMeshes; i-- > 0; )
//						{
//							if (in == scene->mMeshes[i]->mName.C_Str())
//							{
//								index = i;
//								asdfafg = true;
//								break;
//							}
//						}
//						if (!asdfafg)
//						{
//							std::cout << "Invalid Name" << std::endl;
//							continue;
//						}
//					}
//				}
//				std::cout << std::endl;
//				if ((unsigned int)max(0, index) >= scene->mNumMeshes)
//					std::cout << "Index too high" << std::endl;
//				else
//				{
//					if (index < 0)
//						break;
//					targetmesh = scene->mMeshes[index];
//					ListString ass;
//					if (!targetmesh->HasFaces())
//						ass += "Faces";
//					if (!targetmesh->HasPositions())
//						ass += "Positions";
//					if (!targetmesh->HasNormals())
//						ass += "Normals";
//					if (!targetmesh->HasTangentsAndBitangents())
//						(ass += "Tangents") += "Bitangents";
//					if (!targetmesh->HasTextureCoords(0))
//						ass += "Texture Coords";
//
//					constexpr static Matrixy4x4 BlendToNorm{
//						0.f, 0.f, 1.f, 0.f,
//						1.f, 0.f, 0.f, 0.f,
//						0.f, 1.f, 0.f, 0.f,
//						0.f, 0.f, 0.f, 1.f
//					};
//
//					if (ass)
//					{
//						ass.EndingSeparator = " or ";
//						std::cout << "Target mesh does not have: " << ass.Finalize() << " and will not be used";
//					}
//					else
//					{
//						std::cout << "Outputting Vertices..." << std::endl << "constexpr FullVertex Vertices[] = " << std::endl << "{" << std::endl;
//
//						for (size_t i = 0; i < targetmesh->mNumVertices; ++i)
//						{
//							std::cout << "FullVertex({ floaty3{ " << tcs(targetmesh->mVertices[i].y) << ", " << tcs(targetmesh->mVertices[i].z) << ", " << tcs(targetmesh->mVertices[i].x) << " }, floaty3{ "
//								<< tcs(targetmesh->mTangents[i].y) << ", " << tcs(targetmesh->mTangents[i].z) << ", " << tcs(targetmesh->mTangents[i].x) << " }, floaty3{ "
//								<< tcs(targetmesh->mBitangents[i].y) << ", " << tcs(targetmesh->mBitangents[i].z) << ", " << tcs(targetmesh->mBitangents[i].x) << " }, floaty3{ "
//								<< tcs(targetmesh->mNormals[i].y) << ", " << tcs(targetmesh->mNormals[i].z) << ", " << tcs(targetmesh->mNormals[i].x) << " }, floaty2{ "
//								<< tcs(targetmesh->mTextureCoords[0][i].x) << ", " << tcs(targetmesh->mTextureCoords[0][i].y) << " } }), " << std::endl;
//						}
//
//						std::cout << "}" << std::endl << "Outputting Indices..." << std::endl << " constexpr unsigned int Indices[] = { ";
//
//						std::string steve;
//						std::stringstream bob;
//						for (size_t i = 0; i < targetmesh->mNumFaces; ++i)
//						{
//							bob << targetmesh->mFaces[i].mIndices[0] << ", " << targetmesh->mFaces[i].mIndices[1] << ", " << targetmesh->mFaces[i].mIndices[2] << ", ";
//						}
//
//						steve = bob.str();
//						steve.pop_back();
//						steve.pop_back();
//
//
//						std::cout << steve << " }" << std::endl;
//
//						std::cout << "Outputting Material..." << std::endl;
//						std::cout << "constexpr Material mat =" << std::endl;
//						std::cout << "{" << std::endl;
//						auto *mat = scene->mMaterials[targetmesh->mMaterialIndex];
//						aiColor3D color;
//						mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
//						std::cout << "{ " << tcs(color.r) << ", " << tcs(color.g) << ", " << tcs(color.b) << ", 1.f }," << std::endl;
//						mat->Get(AI_MATKEY_COLOR_EMISSIVE, color);
//						std::cout << "{ " << tcs(color.r) << ", " << tcs(color.g) << ", " << tcs(color.b) << ", 1.f }," << std::endl;
//						mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
//						std::cout << "{ " << tcs(color.r) << ", " << tcs(color.g) << ", " << tcs(color.b) << ", ";
//						float val = 1.f;
//						mat->Get(AI_MATKEY_OPACITY, val);
//						std::cout << tcs(val) << " }, " << std::endl;
//						mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
//						std::cout << "{ " << tcs(color.r) << ", " << tcs(color.g) << ", " << tcs(color.b) << ", ";
//						val = 1.f;
//						mat->Get(AI_MATKEY_SHININESS, val);
//						std::cout << tcs(val) << " }," << std::endl;
//						float ior = 1.f;
//						mat->Get(AI_MATKEY_REFRACTI, ior);
//						std::cout << tcs(ior) << "," << std::endl << "0u" << std::endl;
//						aiString name;
//						if (AI_SUCCESS == mat->Get(AI_MATKEY_NAME, name))
//							std::cout << "Material Name is: " << name.C_Str() << std::endl;
//						else
//							std::cout << "Material Unnamed" << std::endl;
//					}
//				}
//			}
//		}
//	}
//}

struct IntermediateNode
{
	std::string Name;
	Matrixy4x4 FlattenedTransform;
	Matrixy4x4 LocalTransform;
	unsigned int MeshRef;
};

struct IntermediateGeo
{
	std::string GeoName;
	unsigned int MaterialIndex;
};

namespace std
{
	template<>
	struct hash<IntermediateGeo>
	{
		inline size_t operator()(const IntermediateGeo &tohash) const
		{
			return ((std::hash<std::string>()(tohash.GeoName) / 2) << (sizeof(size_t) / 2 * 8)) | (size_t)tohash.MaterialIndex;
		}
	};
}

inline bool operator==(const IntermediateGeo &a, const IntermediateGeo& b)
{
	return a.GeoName == b.GeoName && a.MaterialIndex == b.MaterialIndex;
}

void PullNodes2(const aiNode * node, std::vector<IntermediateNode>& nodes)
{
	if (node->mNumMeshes)
	{
		Matrixy4x4 trans = node->mTransformation;
		for (auto *n = node->mParent; n != nullptr; n = n->mParent)
			trans = Matrixy4x4::Multiply(trans, n->mTransformation);
		for (size_t i = node->mNumMeshes; i-- > 0; )
		{
			auto &pnode = nodes.emplace_back();
			pnode.FlattenedTransform = trans;
			pnode.LocalTransform = node->mTransformation;
			pnode.MeshRef = node->mMeshes[i];
			pnode.Name = node->mName.C_Str();
			if (i > 0)
				pnode.Name += std::to_string(i);
		}
	}
	for (size_t i = node->mNumChildren; i-- > 0; )
	{
		PullNodes2(node->mChildren[i], nodes);
	}
}


void PullNodesNoRoot(const aiNode * node, std::vector<IntermediateNode>& nodes)
{
	if (node->mNumMeshes)
	{
		Matrixy4x4 trans = Matrixy4x4::Identity();
		for (auto *n = node; n->mParent != nullptr; n = n->mParent)
			trans = Matrixy4x4::Multiply(trans, n->mTransformation);
		for (size_t i = node->mNumMeshes; i-- > 0; )
		{
			auto &pnode = nodes.emplace_back();
			pnode.FlattenedTransform = trans;
			pnode.LocalTransform = node->mTransformation;
			pnode.MeshRef = node->mMeshes[i];
			pnode.Name = node->mName.C_Str();
			if (i > 0)
				pnode.Name += std::to_string(i);
		}
	}
	for (size_t i = node->mNumChildren; i-- > 0; )
	{
		PullNodesNoRoot(node->mChildren[i], nodes);
	}
}

void PullNodes(const aiNode * node, std::vector<IntermediateNode>& nodes, Matrixy4x4 accumulated = Matrixy4x4::Identity())
{
	accumulated = Matrixy4x4::Multiply(accumulated, node->mTransformation);
	for (size_t i = node->mNumMeshes; i-- > 0; )
	{
		auto &pnode = nodes.emplace_back();
		pnode.FlattenedTransform = accumulated;
		pnode.MeshRef = node->mMeshes[i];
		pnode.Name = node->mName.C_Str();
		if (i > 0)
			pnode.Name += std::to_string(i);
	}
	for (size_t i = node->mNumChildren; i-- > 0; )
	{
		PullNodes(node->mChildren[i], nodes, accumulated);
	}
}

void PullMesh(const aiMesh *mesh, std::unordered_map<IntermediateGeo, PulledGeometry> & geo)
{
	if (!mesh)
		return;

	std::string name = mesh->mName.C_Str();
	if (name.empty())
	{
		name = "Unnamed";
		DWARNING("Unnamed Mesh detected, this will overwrite any previous Unnamed Geometries and Materials");
	}
	if (mesh->HasPositions() && mesh->HasNormals() && mesh->HasTangentsAndBitangents())
	{
		if (mesh->HasTextureCoords(0))
		{
			if (mesh->HasFaces())
			{
				std::vector<FullVertex> vertices;
				for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
				{
					auto &vertex = vertices.emplace_back();
					vertex.PosL = floaty3(mesh->mVertices[i]);
					vertex.NormalL = floaty3(mesh->mNormals[i]);
					vertex.TangentL = floaty3(mesh->mTangents[i]);
					vertex.BinormalL = floaty3(mesh->mBitangents[i]);
					vertex.Tex = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
				}
				std::vector<unsigned int> indices;
				indices.reserve(mesh->mNumFaces * 3u);
				for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
				{
					indices.emplace_back(mesh->mFaces[i].mIndices[0]);
					indices.emplace_back(mesh->mFaces[i].mIndices[1]);
					indices.emplace_back(mesh->mFaces[i].mIndices[2]);
				}
				if (geo.find({ name, mesh->mMaterialIndex }) != geo.end())
					DWARNING("Duplicate Geometry + Material combo found overwriting...");
				geo.emplace(IntermediateGeo{ name, mesh->mMaterialIndex }, PulledGeometry{ std::move(vertices), std::move(indices) });
			}
			else
			{
				DWARNING(name + " Geometry does not have indices");
			}
		}
		else
		{
			DWARNING(name + " Geometry Does not have Texture Coordinates");
		}
	}
	else
	{
		if (!mesh->HasTextureCoords(0))
			DWARNING(name + " Geometry does not have Texture Coordinates required to generate tangents and bitangents");
		else
			DWARNING(name + " Geometry does not have vertices");
	}
}

void PullMeshInvertRoot(const aiMesh *mesh, std::unordered_map<IntermediateGeo, PulledGeometry> & geo, Matrixy4x4 RootInverse)
{
	if (!mesh)
		return;

	std::string name = mesh->mName.C_Str();
	if (name.empty())
	{
		name = "Unnamed";
		DWARNING("Unnamed Mesh detected, this will overwrite any previous Unnamed Geometries and Materials");
	}
	if (mesh->HasPositions() && mesh->HasNormals() && mesh->HasTangentsAndBitangents())
	{
		if (mesh->HasTextureCoords(0))
		{
			if (mesh->HasFaces())
			{
				std::vector<FullVertex> vertices;
				for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
				{
					auto &vertex = vertices.emplace_back();
					vertex.PosL = RootInverse.Transform(floaty3(mesh->mVertices[i]));
					vertex.NormalL = RootInverse.TransformNormal(floaty3(mesh->mNormals[i]));
					vertex.TangentL = RootInverse.TransformNormal(floaty3(mesh->mTangents[i]));
					vertex.BinormalL = RootInverse.TransformNormal(floaty3(mesh->mBitangents[i]));
					vertex.Tex = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
				}
				std::vector<unsigned int> indices;
				indices.reserve(mesh->mNumFaces * 3u);
				for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
				{
					indices.emplace_back(mesh->mFaces[i].mIndices[0]);
					indices.emplace_back(mesh->mFaces[i].mIndices[1]);
					indices.emplace_back(mesh->mFaces[i].mIndices[2]);
				}
				if (geo.find({ name, mesh->mMaterialIndex }) != geo.end())
					DWARNING("Duplicate Geometry + Material combo found overwriting...");
				geo.emplace(IntermediateGeo{ name, mesh->mMaterialIndex }, PulledGeometry{ std::move(vertices), std::move(indices) });
			}
			else
			{
				DWARNING(name + " Geometry does not have indices");
			}
		}
		else
		{
			DWARNING(name + " Geometry Does not have Texture Coordinates");
		}
	}
	else
	{
		if (!mesh->HasTextureCoords(0))
			DWARNING(name + " Geometry does not have Texture Coordinates required to generate tangents and bitangents");
		else
			DWARNING(name + " Geometry does not have vertices");
	}
}

Material PullMaterial(const aiMaterial *mat, std::string &out)
{
	Material outm;
	outm.TextureBitmap = 0u;
	aiString name;
	mat->Get(AI_MATKEY_NAME, name);
	out = name.C_Str();

	aiColor3D color = { 0.f, 0.f, 0.f };
	float val = 1.f;
	mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
	outm.AmbientColor = { color.r, color.g, color.b, 1.f };
	
	color = { 0.f, 0.f, 0.f };
	mat->Get(AI_MATKEY_COLOR_EMISSIVE, color);
	outm.EmissiveColor = { color.r, color.g, color.b, 1.f };
	
	color = { 0.f, 0.f, 0.f };
	val = 1.f;
	mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
	mat->Get(AI_MATKEY_OPACITY, val);
	outm.DiffuseColor = { color.r, color.g, color.b, val };

	color = { 0.f, 0.f, 0.f };
	val = 1.f;
	mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
	mat->Get(AI_MATKEY_SHININESS, val);
	outm.SpecularColor = { color.r, color.g, color.b, val };

	val = 1.f;
	mat->Get(AI_MATKEY_REFRACTI, val);
	outm.IndexOfRefraction = val;
	return outm;
}

PulledScene PullScene(const aiScene * scene, PullOptions options)
{
	assert(scene);

	if (!scene)
		return PulledScene();

	DINFO("Pulling Assimp Scene");
	PulledScene out;
	out.Name = scene->mRootNode->mName.C_Str();

	// Manually Modify the root transform to rotate around the Y Axis 90* (otherwise geometry is not inline with axises (by 90*))
	Matrixy4x4 tmp = scene->mRootNode->mTransformation;
	tmp = Matrixy4x4::MultiplyE(Matrixy4x4::Transpose(Matrixy3x3::RotationY(-Math::NinetyDegreesRF)), tmp);
	std::copy(tmp.ma, tmp.ma + 16, &scene->mRootNode->mTransformation.a1);
	out.RootTransform = Matrixy4x4::Transpose(tmp);

	Matrixy4x4 InvertedRoot;
	if (options.RootTreatment == PullOptions::Get_rid_of_it)
	{
		if (!Matrixy4x4::Inverted(out.RootTransform, InvertedRoot))
			options.RootTreatment = PullOptions::Put_up_with_it;
	}

	std::unordered_map<unsigned int, std::string> MatIndexToName;
	MatIndexToName.reserve(scene->mNumMaterials);
	out.Materials.reserve(scene->mNumMaterials);
	for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	{
		std::string name;
		Material mat = PullMaterial(scene->mMaterials[i], name);
		if (out.Materials.find(name) != out.Materials.end())
			DWARNING("Duplicate Material Found");
		out.Materials[name] = mat;
		MatIndexToName[i] = name;
	}
	
	std::unordered_map<IntermediateGeo, PulledGeometry> Geos;
	Geos.reserve(scene->mNumMeshes);
	if (options.RootTreatment == PullOptions::Get_rid_of_it)
	{
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			PullMeshInvertRoot(scene->mMeshes[i], Geos, out.RootTransform);
		}
	}
	else
	{
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			PullMesh(scene->mMeshes[i], Geos);
		}
	}

	out.Geometries.reserve(Geos.size());
	for (auto &geo : Geos)
	{
		GeoMatCombo geoname = { geo.first.GeoName, MatIndexToName[geo.first.MaterialIndex] };
		out.Geometries[geoname] = geo.second;
	}

	std::vector<IntermediateNode> nodes;
	if (options.RootTreatment == PullOptions::Get_rid_of_it || options.RootTreatment == PullOptions::Fucking_ignore_it)
		PullNodesNoRoot(scene->mRootNode, nodes);
	else
		PullNodes2(scene->mRootNode, nodes);// , scene->mRootNode->mTransformation);

	std::vector<aiMesh*> Meshes;
	Meshes.resize(scene->mNumMeshes);
	std::copy_n(scene->mMeshes, scene->mNumMeshes, Meshes.begin());

	out.Nodes.reserve(nodes.size());
	for (size_t i = nodes.size(); i-- > 0; )
	{
		PulledNode node;
		node.FlattenedTransform = /*Matrixy4x4::MultiplyE(*/Matrixy4x4::Transpose(nodes[i].FlattenedTransform);/* , Matrixy4x4::AssimpTrans());*/
		node.LocalTransform = Matrixy4x4::Transpose(nodes[i].LocalTransform);
		node.NodeName = nodes[i].Name;
		node.GeoName = scene->mMeshes[nodes[i].MeshRef]->mName.C_Str();
		node.MaterialName = MatIndexToName[scene->mMeshes[nodes[i].MeshRef]->mMaterialIndex];
		out.Nodes.emplace(node);
	}
	
	for (size_t i = scene->mNumLights; i-- > 0; )
	{
		auto *alight = scene->mLights[i];
		aiNode *lightnode = scene->mRootNode->FindNode(alight->mName);
		if (!lightnode)
			continue;

		Matrixy4x4 trans = lightnode->mTransformation;
		for (auto *p = lightnode->mParent; p; p = p->mParent)
			trans = Matrixy4x4::Multiply(trans, p->mTransformation);
		trans = Matrixy4x4::Transpose(trans);
		auto it = LightMap.find(alight->mType);
		if (it != LightMap.end())
		{
			auto &light = out.Lights[(std::string)alight->mName.C_Str()];
			light.Color = { alight->mColorDiffuse.r, alight->mColorDiffuse.g, alight->mColorDiffuse.b };
			light.Type = it->second;
			light.Enabled = true;
			if (light.Type == LIGHT_SPOT || light.Type == LIGHT_POINT)
			{
				auto &constantf = alight->mAttenuationConstant;
				auto &linf = alight->mAttenuationLinear;
				auto &quadf = alight->mAttenuationQuadratic;
				light.Attenuation = { constantf, linf, quadf };
				float at = 0.5f;
				float a = at * quadf;
				float b = at * linf;
				float c = at * constantf - 1.f;
				float root = b * b - 4 * a * c;
				if (root > 0.f)
				{
					float distance1 = (-b + sqrt(root)) / (2.f * a);
					float distance2 = (-b - sqrt(root)) / (2.f * a);
					light.Range = fmaxf(distance1, distance2);
				}
				else
					light.Range = 0.f;
				
				light.PositionWS = trans.Transform({ alight->mPosition.x, alight->mPosition.y, alight->mPosition.z, 1.f });
				if (light.Type == LIGHT_SPOT)
				{
					light.SpotlightAngle = alight->mAngleOuterCone;
					light.DirectionWS = trans.TransformNormal({ alight->mDirection.x, alight->mDirection.y, alight->mDirection.z, 1.f });
				}
			}
			else
			{
				light.DirectionWS = { alight->mDirection.x, alight->mDirection.y, alight->mDirection.z, 1.f };
			}
		}
	}

	return out;
}
