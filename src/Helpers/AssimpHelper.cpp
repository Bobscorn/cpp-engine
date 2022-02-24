
//#include "Thing.h"

#include "AssimpHelper.h"
#ifdef USE_ASSIMP
#include "StringHelper.h"
#include "EnumHelper.h"

#include "Graphics.h"

XMFLOAT4 AssimpHelper::aiToXM(const aiColor4D & col)
{
	return XMFLOAT4(col.r, col.g, col.b, col.a);
}

XMFLOAT3 AssimpHelper::aiToXM(const aiColor3D & col)
{
	return XMFLOAT3(col.r, col.g, col.b);
}

XMFLOAT3 AssimpHelper::aiToXM(const aiVector3D & vec)
{
	return XMFLOAT3(vec.x, vec.y, vec.z);
}

XMFLOAT2 AssimpHelper::aiToXM(const aiVector2D & vec)
{
	return XMFLOAT2(vec.x, vec.y);
}

aiNode * AssimpHelper::FindNodeByName(const std::wstring &name, const aiNode * in)
{
	return FindNodeByName((aiString)StringHelper::WtoC(name).c_str(), in);
}

aiNode * AssimpHelper::FindNodeByName(const char * name, const aiNode * in)
{
	return FindNodeByName((aiString)name, in);
}

aiNode * AssimpHelper::FindNodeByName(const aiString &name, const aiNode *in, XMFLOAT4X4 *trans)
{
	aiNode *node = nullptr;
	// First search the current node
	for (UINT i = in->mNumChildren; i-- > 0; )
	{
		if (in->mChildren[i]->mName == name)
		{
			node = in->mChildren[i];
			break;
		}
	}

	// Then search their children if not found
	if (!node)
	{
		for (UINT i = in->mNumChildren; i-- > 0 && !node; )
		{
			node = FindNodeByName(name, in->mChildren[i]);
		}
	}
	return node;
}



void AssimpHelper::TraverseMeshNode(std::vector<IntermediateShapeNode>& tup, aiNode * toTraverse, int &shapeindex)
{
	int mindex = shapeindex;
	// Go through children's meshes
	for (UINT i = toTraverse->mNumChildren; i-- > 0; )
	{
		aiNode *child = toTraverse->mChildren[i];

		size_t childindex = tup.size(); // Index into tup of the current child (hasn't been added yet, so it'll be the current size of the tup)

										// This node, or a node underneath it contains meshes so add a shape for this node to keep the hierachy/chain
		if (NodeOrChildrenContainMesh(toTraverse->mChildren[i]))
		{
			++shapeindex; // Move the index to match this child's index

						  // Create a blank Shape in the tup
			tup.emplace_back();
			IntermediateShapeNode *childy = &tup.back();

			// Name
			// v
			childy->NodeName = StringHelper::AtoW(child->mName);
			// ^
			// Name
			// Index and Parent Index
			// v
			childy->Index = shapeindex;
			childy->parentIndex = mindex;
			// ^
			// Index and Parent Index
			// Matrix
			// v
			childy->NodeToParent = GetLocalMatrix(child);
			// ^
			// Matrix
			// Mesh Indices
			// v
			if (toTraverse->mChildren[i]->mNumMeshes)
				childy->MeshIndices = std::vector<size_t>(child->mMeshes, child->mMeshes + child->mNumMeshes);
			// ^
			// Mesh Indices
			// Child Indices (computed by counting Shapes that need to be created)
			// v
			size_t offset = 1; // First child is 1 off this shape
			for (UINT j = child->mNumChildren; j-- > 0; )
			{
				childy->ChildrenIndices.push_back(childindex + offset);
				offset += CountShapesInNode(child->mChildren[j]);
			}
			// ^
			// Child Indices
		}
		else
			continue;

		TraverseMeshNode(tup, child, shapeindex);
	}
}

bool AssimpHelper::NodeOrChildrenContainMesh(aiNode * node)
{
	// Check if this node has meshes
	if (node->mNumMeshes)
		return true;

	// If not, check the children for meshes, using recursion
	bool childhasmesh = false;
	for (UINT i = node->mNumChildren; i-- > 0 && !childhasmesh; )
	{
		childhasmesh |= NodeOrChildrenContainMesh(node->mChildren[i]);
	}

	// By now it'll know if there are any meshes
	return childhasmesh;
}

bool AssimpHelper::ChildrenContainMesh(aiNode * node)
{
	bool gotmesh = false;

	// First check if any children have meshes
	for (UINT i = node->mNumChildren; i-- > 0 && !gotmesh; )
	{
		gotmesh |= (bool)node->mChildren[i]->mNumMeshes;
	}

	// If not, check if their children have meshes using recursion
	if (!gotmesh)
	{
		for (UINT i = node->mNumChildren; i-- > 0 && !gotmesh; )
		{
			gotmesh |= ChildrenContainMesh(node->mChildren[i]);
		}
	}

	// By now it'll know if there are any meshes
	return gotmesh;
}

size_t AssimpHelper::CountShapesInNode(aiNode * node)
{
	// If this node or its children contain meshes, count this node as it will be needed to keep the chain
	size_t count = 0;
	if (NodeOrChildrenContainMesh(node))
		++count;

	// Check its children, if it has none then recursion will stop
	for (size_t i = node->mNumChildren; i-- > 0; )
	{
		// Add 1 to count if a child has meshes
		count += CountShapesInNode(node->mChildren[i]);
	}
	return count;
}


std::pair<XMMATRIX, std::wstring> AssimpHelper::AddBones(std::map<std::wstring, SpecialBone>& map, aiNode * node
#ifdef CULL_WEIGHTLESS_ROOT_BONES
	, std::function<bool(std::wstring)> HasWeights
#endif
	, DirectX::XMFLOAT4X4 ass, std::vector<Graphics::VertexWeight> weights, bool IsBone)
{
	std::wstring name = StringHelper::AtoW(node->mName);
	XMMATRIX RootToMe;
	bool assy = map.find(name) == map.end();
	if (assy)
	{
		AssimpHelper::SpecialBone boney;
		boney.Name = name;
		boney.Weights = weights;

		RootToMe = AssimpHelper::aiMatrixtoXMMatrix(node->mTransformation);

		// Make Parent first, except the scene node
#ifdef CULL_WEIGHTLESS_ROOT_BONES
		if (node->mParent && !node->mParent->mParent && NeedsNodeAsBone(node, HasWeights))
		{
			std::pair<XMMATRIX, std::wstring> pear = AddBones(map, node->mParent, HasWeights);
#else
		if (node->mParent && node->mParent->mParent)
		{
			std::pair<XMMATRIX, std::wstring> pear = AddBones(map, node->mParent);
#endif
			boney.Parent = pear.second;
			RootToMe = XMMatrixMultiply(pear.first, RootToMe);
			map[boney.Parent].Children.push_back(boney.Name);
		}
		else
			boney.Parent = L"NO PARENT";

		if (ass == XMFLOAT4X4())
			XMStoreFloat4x4(&boney.ObjectToBoneTransform, RootToMe);

		boney.OffsetMatrix = AssimpHelper::aiMatrixToXMFloat(node->mTransformation);

		map.emplace(std::make_pair(boney.Name, boney));
		}

	if (weights.size())
		map[name].Weights = weights;

	// ass will be blank if its not a bone (just a step from one bone to another)
	if (ass != XMFLOAT4X4())
		map[name].ObjectToBoneTransform = ass;

	if (IsBone)
		map[name].IsBone = true;

	if (!assy)
		RootToMe = XMLoadFloat4x4(&map[name].ObjectToBoneTransform);

	return std::make_pair(RootToMe, name);
	}

bool AssimpHelper::NeedsNodeAsBone(aiNode * Node, std::function<bool(std::wstring)> HasWeights)
{
	for (aiNode *nodey = Node; nodey != nullptr; nodey = nodey->mParent)
	{
		if (HasWeights(StringHelper::AtoW(nodey->mName)))
			return true;
	}
	return false;
}


UINT AssimpHelper::BoneThingy(std::vector<Graphics::Bone> *bvec, std::map<std::wstring, SpecialBone> &bonemap, SpecialBone sBone)
{
	// Make the bone
	bvec->emplace_back();
	UINT ParentIndex = (UINT)bvec->size() - 1u;

	// Make Children, and make their parents this
	std::vector<Graphics::Bone*> bonesies;
	bonesies.reserve(sBone.Children.size());
	for (UINT i = (UINT)sBone.Children.size(); i-- > 0; )
	{
		AssimpHelper::SpecialBone sbonez = bonemap[sBone.Children[i]];
		UINT index = BoneThingy(bvec, bonemap, sbonez);
		bonesies.push_back(&(*bvec)[index]);
		(*bvec)[index].Parent = &(*bvec)[ParentIndex];
	}

	Graphics::Bone &bone = bvec->operator[](ParentIndex);
	bone.Children = bonesies;
	bone.Name = sBone.Name;
	bone.RootToBone = sBone.ObjectToBoneTransform;
	bone.Weights = sBone.Weights;

	return ParentIndex;
}

Geometry::Material AssimpHelper::aiToMaterial(aiMaterial *aimat)
{
	aiColor3D Diffuse;
	aimat->Get(AI_MATKEY_COLOR_DIFFUSE, Diffuse);

	aiColor3D Specular;
	aimat->Get(AI_MATKEY_COLOR_SPECULAR, Specular);

	aiColor3D Ambient;
	aimat->Get(AI_MATKEY_COLOR_AMBIENT, Ambient);

	aiColor3D Emissive;
	aimat->Get(AI_MATKEY_COLOR_EMISSIVE, Emissive);

	aiColor3D Transparent;
	aimat->Get(AI_MATKEY_COLOR_TRANSPARENT, Transparent);

	float IOR;
	aimat->Get(AI_MATKEY_REFRACTI, IOR);

	float Opacity;
	aimat->Get(AI_MATKEY_OPACITY, Opacity);

	if (Opacity < 0.f)
		Opacity = 1.f;

	float SpecPow;
	aimat->Get(AI_MATKEY_SHININESS, SpecPow);

	Geometry::Material mat;

	mat.DiffuseColor.x = Diffuse.r;
	mat.DiffuseColor.y = Diffuse.g;
	mat.DiffuseColor.z = Diffuse.b;
	mat.DiffuseColor.w = Opacity;

	mat.AmbientColor.x = Ambient.r;
	mat.AmbientColor.y = Ambient.g;
	mat.AmbientColor.z = Ambient.b;
	mat.AmbientColor.w = 1.0f;

	mat.EmissiveColor.x = Emissive.r;
	mat.EmissiveColor.y = Emissive.g;
	mat.EmissiveColor.z = Emissive.b;
	mat.EmissiveColor.w = 1.0f;

	mat.IndexOfRefraction = IOR;

	mat.SpecularColor.x = Specular.r;
	mat.SpecularColor.y = Specular.g;
	mat.SpecularColor.z = Specular.b;
	mat.SpecularColor.w = SpecPow;

	return mat;
}



XMMATRIX AssimpHelper::GetWorldRotationMatrixXM(aiNode * node)
{
	XMMATRIX mat = XMMatrixIdentity();

	for (aiNode *nodey = node; nodey; nodey = nodey->mParent)
	{
		XMFLOAT3X3 Rot;
		XMStoreFloat3x3(&Rot, AssimpHelper::aiMatrixtoXMMatrix(nodey->mTransformation));
		mat = XMMatrixMultiply(mat, XMLoadFloat3x3(&Rot));
	}

	return mat;
}

XMFLOAT4X4 AssimpHelper::GetWorldRotationMatrix(aiNode * node)
{
	XMFLOAT4X4 mat;
	XMStoreFloat4x4(&mat, GetWorldRotationMatrixXM(node));
	return mat;
}

XMFLOAT4X4 AssimpHelper::GetWorldMatrix(aiNode * node)
{
	XMFLOAT4X4 ass;
	XMStoreFloat4x4(&ass, GetWorldMatrixXM(node));
	return ass;
}

XMFLOAT4X4 AssimpHelper::GetLocalMatrix(aiNode *node)
{
	return AssimpHelper::aiMatrixToXMFloat(node->mTransformation);
}

XMMATRIX AssimpHelper::GetWorldMatrixXM(aiNode * node)
{
	XMMATRIX matrix = XMMatrixIdentity();

	// Get the number of parents (distance from rootnode)
	for (aiNode *rootNode = node; rootNode; rootNode = rootNode->mParent)
	{
		matrix = XMMatrixMultiply(matrix, AssimpHelper::aiMatrixtoXMMatrix(rootNode->mTransformation));
	}

	return matrix;
}



XMFLOAT4X4 AssimpHelper::aiMatrixToXMFloat(aiMatrix4x4 & matrix)
{
	return XMFLOAT4X4(matrix.a1, matrix.b1, matrix.c1, matrix.d1, matrix.a2, matrix.b2, matrix.c2, matrix.d2, matrix.a3, matrix.b3, matrix.c3, matrix.d3, matrix.a4, matrix.b4, matrix.c4, matrix.d4);
}

XMMATRIX AssimpHelper::aiMatrixtoXMMatrix(aiMatrix4x4 & matrix)
{
	return XMMatrixTranspose(XMMATRIX(matrix.a1, matrix.a2, matrix.a3, matrix.a4, matrix.b1, matrix.b2, matrix.b3, matrix.b4, matrix.c1, matrix.c2, matrix.c3, matrix.c4, matrix.d1, matrix.d2, matrix.d3, matrix.d4));
}

XMMATRIX AssimpHelper::aiMatrixtoXMMatrix(aiMatrix3x3 & matrix)
{
	return XMMatrixTranspose(XMMATRIX(matrix.a1, matrix.a2, matrix.a3, 0.f, matrix.b1, matrix.b2, matrix.b3, 0.f, matrix.c1, matrix.c2, matrix.c3, 0.f, 0.f, 0.f, 0.f, 1.f));
}



AssimpHelper::LightShit AssimpHelper::GetLightShitFromName(std::wstring name)
{
	// Lights should be named as follows
	// [ColorType]Name
	LightShit lighty;
	size_t ass = 0;
	lighty.Type = Enum::FirstFindEnum(name, Geometry::ColorSearchything, &ass);
	lighty.Name = std::wstring(name.begin() + ass, name.end());
	return lighty;
}


#else

int adfarg = 2;

#endif
