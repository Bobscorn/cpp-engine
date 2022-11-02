#include "SimpleMeshImport.h"

#include "Helpers/DebugHelper.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/config.h>

// Returns false if something is wrong, otherwise true
const aiScene* InitializeSimpleImporter(Assimp::Importer& importer, const std::string& fileName)
{
	// Since we only care about the mesh data,
	// This configures the importer to exclude Animations, Boneweights, Cameras, Lights, Materials and textures
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
		aiComponent_ANIMATIONS |
		aiComponent_BONEWEIGHTS |
		aiComponent_CAMERAS |
		aiComponent_LIGHTS |
		aiComponent_MATERIALS |
		aiComponent_TEXTURES);

	// Here we set the aiProcess_RemoveComponent flag telling the importer to use the above property to exclude those components
	// We also tell it to triangulate all meshes (if it's not already)
	auto* scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_RemoveComponent | aiProcess_CalcTangentSpace);

	if (!scene)
	{
		DWARNING("Trying to import mesh '" + fileName + "' Assimp failed!");
		if (auto err = importer.GetErrorString(); err)
			DWARNING("Assimp error is: " + err);
		else
			DWARNING("Assimp has no error!");
		DWARNING("An empty mesh will be returned instead");
		return nullptr;
	}

	if (!scene->HasMeshes() || !scene->mNumMeshes)
	{
		DWARNING("Trying to import mesh '" + fileName + "' no meshes were found!");
		DWARNING("An empty mesh will be returned instead");
		return nullptr;
	}

	return scene;
}

Importing::SimpleMesh Importing::ImportSimpleMesh(const std::string& fileName)
{
	auto importer = Assimp::Importer();

	auto scene = InitializeSimpleImporter(importer, fileName);
	if (!scene)
		return { {}, {} };

	// Then we simply grab the first mesh
	SimpleMesh mesh;
	auto* assimpMesh = scene->mMeshes[0];

	if (!(assimpMesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE))
	{
		DWARNING("Trying to import mesh '" + fileName + "' the first mesh does not use triangles!");
		DWARNING("An empty mesh will be returned instead");
		return { {}, {} };
	}

	mesh.Vertices.reserve((size_t)assimpMesh->mNumVertices);
	for (unsigned int i = 0; i < assimpMesh->mNumVertices; ++i)
	{
		auto& aVert = assimpMesh->mVertices[i];
		FullVertex vert;
		vert.PosL = { aVert.x, aVert.y, aVert.z };
		vert.NormalL = { 0.f, 0.f, 0.f };
		if (assimpMesh->HasNormals())
		{
			auto& norm = assimpMesh->mNormals[i];
			vert.NormalL = { norm.x, norm.y, norm.z };
		}
		vert.TangentL = { 0.f, 0.f, 0.f };
		vert.BinormalL = { 0.f, 0.f, 0.f };
		if (assimpMesh->HasTangentsAndBitangents())
		{
			auto& bin = assimpMesh->mBitangents[i];
			auto& tan = assimpMesh->mTangents[i];
			vert.BinormalL = { bin.x, bin.y, bin.z };
			vert.TangentL = { tan.x, tan.y, tan.z };
		}
		vert.Tex = { 0.f, 0.f };
		if (assimpMesh->HasTextureCoords(0))
		{
			auto& aiTex = assimpMesh->mTextureCoords[0][i];
			vert.Tex = { aiTex.x, aiTex.y };
		}
		mesh.Vertices.push_back(vert);
	}

	bool containsNonTris = false;
	mesh.Indices.reserve((size_t)assimpMesh->mNumFaces * 3);
	for (unsigned int i = 0; i < assimpMesh->mNumFaces; ++i)
	{
		auto& face = assimpMesh->mFaces[i];
		if (face.mNumIndices != 3)
		{
			containsNonTris = true;
			continue;
		}
		mesh.Indices.push_back(face.mIndices[0]);
		mesh.Indices.push_back(face.mIndices[1]);
		mesh.Indices.push_back(face.mIndices[2]);
	}

	return mesh;
}

std::vector<Importing::SimpleMesh> Importing::ImportAllSimpleMeshes(const std::string& fileName)
{
	auto imp = Assimp::Importer();

	auto scene = InitializeSimpleImporter(imp, fileName);

	if (!scene)
		return {};

	std::vector<Importing::SimpleMesh> meshes{};
	meshes.reserve(scene->mNumMeshes);
	for (unsigned int meshI = 0; meshI < scene->mNumMeshes; ++meshI)
	{
		auto* aiMesh = scene->mMeshes[meshI];
		SimpleMesh mesh;
		mesh.MeshName = aiMesh->mName.C_Str();

		mesh.Vertices.reserve(aiMesh->mNumVertices);
		for (unsigned int vertI = 0; vertI < aiMesh->mNumVertices; ++vertI)
		{
			auto& aiVert = aiMesh->mVertices[vertI];
			FullVertex vert;
			vert.PosL = floaty3{ aiVert.x, aiVert.y, aiVert.z };
			vert.NormalL = { 0.f, 0.f, 0.f };
			if (aiMesh->HasNormals())
			{
				auto& aiNorm = aiMesh->mNormals[vertI];
				vert.NormalL = { aiNorm.x, aiNorm.y, aiNorm.z };
			}
			vert.BinormalL = { 0.f, 0.f, 0.f };
			vert.TangentL = { 0.f, 0.f, 0.f };
			if (aiMesh->HasTangentsAndBitangents())
			{
				auto& aiTan = aiMesh->mTangents[vertI];
				auto& aiBin = aiMesh->mBitangents[vertI];
				vert.TangentL = { aiTan.x, aiTan.y, aiTan.z };
				vert.BinormalL = { aiBin.x, aiBin.y, aiBin.z };
			}
			vert.Tex = { 0.f, 0.f };
			if (aiMesh->HasTextureCoords(0))
			{
				auto& aiTex = aiMesh->mTextureCoords[0][vertI];
				vert.Tex = { aiTex.x, aiTex.y };
			}
			mesh.Vertices.push_back(vert);
		}

		mesh.Indices.reserve(aiMesh->mNumFaces * 3);
		bool hasNonTris = false;
		for (unsigned int faceI = 0; faceI < aiMesh->mNumFaces; ++faceI)
		{
			auto& face = aiMesh->mFaces[faceI];
			if (face.mNumIndices != 3)
			{
				hasNonTris = true;
				continue;
			}

			mesh.Indices.push_back(face.mIndices[0]);
			mesh.Indices.push_back(face.mIndices[1]);
			mesh.Indices.push_back(face.mIndices[2]);
		}

		meshes.emplace_back(std::move(mesh));
	}

	return meshes;
}


