#pragma once

#ifdef USE_ASSIMP

#include <string>
#ifdef WIN32
#include <DirectXMath.h>
using namespace DirectX;
#endif
#include <functional>

#include <assimp/scene.h>

#include "Graphics.h"
#include "Geometry.h"
#include "StorageHeaders.h"


namespace AssimpHelper
{
	typedef unsigned int UINT;

	// Contains matching pairs of Assimp's Light Type Enumeration and My Light Type Enumeration (different from Engine42's unique 'color type' property)
	const std::vector<std::pair<aiLightSourceType, UINT>> LightTypeMatch =
	{
		std::make_pair(aiLightSource_POINT, LIGHT_POINT),
		std::make_pair(aiLightSource_SPOT, LIGHT_SPOT),
		std::make_pair(aiLightSource_DIRECTIONAL, LIGHT_DIRECTION)
	};

	struct LightShit
	{
		std::string Name;
		UINT Type;
	};



	// Obtain Non-Name information from a Light's name
	// This is used for values that are loaded by assimp, but are still required by the app
	LightShit GetLightShitFromName(std::string name);



	Geometry::Material aiToMaterial(aiMaterial *aimat);
	// ^
	// Assimp to Engine42 Material Conversion
	// Assimp to DirectX Class Functions
	// v
#ifdef WIN32
	XMFLOAT4 aiToXM(const aiColor4D &col);
	XMFLOAT3 aiToXM(const aiColor3D &col);
	XMFLOAT3 aiToXM(const aiVector3D &vec);
	XMFLOAT2 aiToXM(const aiVector2D &vec);

	// Goes through the node's parent's matrices and accumulates 
	// the complete matrix from rootnode to the given child node
	XMMATRIX GetWorldRotationMatrixXM(aiNode *node);
	XMFLOAT4X4 GetWorldRotationMatrix(aiNode *node);
	XMFLOAT4X4 GetWorldMatrix(aiNode *node);
	XMFLOAT4X4 GetLocalMatrix(aiNode * node);
	XMMATRIX GetWorldMatrixXM(aiNode *node);

	// Convert aiMatrix4x4 to DirectX::XMFLOAT4X4
	XMFLOAT4X4 aiMatrixToXMFloat(aiMatrix4x4 &matrix);

	// Convert aiMatrix4x4 to DirectX::XMMATRIX
	XMMATRIX aiMatrixtoXMMatrix(aiMatrix4x4 &matrix);

	XMMATRIX aiMatrixtoXMMatrix(aiMatrix3x3 &matrix);

	// ^
	// Assimp to DirectX Class Functions
	// Assimp Node Searching Functions (All 3 do exactly the same thing (2 simply pass it through to the other))
	// v

	// Search a node and its children for a specifically named node
	aiNode * FindNodeByName(const aiString &name, const aiNode *in, XMFLOAT4X4 *trans = nullptr);
#endif // WIN32
	aiNode * FindNodeByName(const std::wstring &name, const aiNode *in);
	aiNode * FindNodeByName(const char *name, const aiNode *in);

	// ^
	// Assimp Node Searching
	// Assimp To Engine42 Structure Conversion Classes
	// v
#ifdef WIN32
	struct IntermediateShapeNode
	{
		XMFLOAT4X4 NodeToParent;
		std::vector<size_t> MeshIndices;
		std::wstring NodeName;
		std::vector<size_t> ChildrenIndices;
		size_t Index;
		INT parentIndex;
	};

	struct SpecialBone
	{
		std::wstring Name;
		std::wstring Parent;
		bool IsBone = false;
		std::vector<std::wstring> Children;
		DirectX::XMFLOAT4X4 ObjectToBoneTransform;
		DirectX::XMFLOAT4X4 OffsetMatrix;
		std::vector<Graphics::VertexWeight> Weights;
	};

	// Go through node's children looking for any that have meshes, 
	// And keep a rootnode-->current node transformation matrix
	// Assume the matrix given is RootNode-->toTraverse

											// Recursive Bonifying function that I Can't really explain
	std::pair<XMMATRIX, std::wstring> AddBones(std::map<std::wstring, SpecialBone> &map, aiNode* node
#ifdef CULL_WEIGHTLESS_ROOT_BONES
		, std::function<bool(std::wstring)> HasWeights
#endif
		, DirectX::XMFLOAT4X4 ass = DirectX::XMFLOAT4X4(), std::vector<Graphics::VertexWeight> weights = std::vector<Graphics::VertexWeight>(), bool IsBone = false);
#endif // WIN32
	bool NeedsNodeAsBone(aiNode *Node, std::function<bool(std::wstring)> HasWeights);

	UINT BoneThingy(std::vector<Graphics::Bone> *bvec, std::map<std::wstring, SpecialBone> &bonemap, SpecialBone sBone);


	void TraverseMeshNode(std::vector<IntermediateShapeNode>& tup, aiNode *toTraverse, int &meshindex);
	bool NodeOrChildrenContainMesh(aiNode *node);
	bool ChildrenContainMesh(aiNode *node);
	size_t CountShapesInNode(aiNode *node); // Recursively counts the number of Mesh containing nodes this node is a parent of
}



#else
struct useless
{
	int ass;
};

#endif