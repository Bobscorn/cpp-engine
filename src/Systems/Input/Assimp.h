#pragma once

#include "Helpers/VectorHelper.h"

#include "Drawing/Graphics3D.h"

#include <assimp/scene.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>


//void DoThing();


struct PulledGeometry
{
	std::vector<FullVertex> Vertices;
	std::vector<unsigned int> Indices;
};

struct PulledNode
{
	std::string NodeName;
	std::string GeoName;
	std::string MaterialName;
	Matrixy4x4 FlattenedTransform;
	Matrixy4x4 LocalTransform;
};

struct PulledMaterial
{
	std::string Name;
	Material Mat;
};

struct PulledNodeHash
{
	size_t inline operator()(const PulledNode &a) const
	{
		return std::hash<std::string>()(a.NodeName);
	}
};

struct PulledNodeEqual
{
	bool inline operator()(const PulledNode &a, const PulledNode &b) const
	{
		return a.NodeName == b.NodeName;
	}
};

struct GeoMatCombo
{
	std::string GeoName;
	std::string MatName;

	inline bool operator==(const GeoMatCombo &other)
	{
		return (GeoName == other.GeoName) && (MatName == other.MatName);
	}
};

inline bool operator==(const GeoMatCombo &a, const GeoMatCombo &b)
{
	return a.GeoName == b.GeoName && a.MatName == b.MatName;
}

namespace std
{
	template<>
	struct hash<GeoMatCombo>
	{
		inline size_t operator()(const GeoMatCombo &tohash) const
		{
			return std::hash<std::string>()(tohash.GeoName + tohash.MatName);
		}
	};
}

struct PulledScene
{
	std::string Name;
	std::unordered_multiset<PulledNode, PulledNodeHash, PulledNodeEqual> Nodes;
	std::unordered_map<GeoMatCombo, PulledGeometry> Geometries;
	std::unordered_map<std::string, Material> Materials;
	std::unordered_map<std::string, Light> Lights;
	Matrixy4x4 RootTransform;
};

struct PullOptions
{
	enum RootNodeTreatment
	{
		Put_up_with_it = 0,
		Get_rid_of_it = 1,
		Fucking_ignore_it = 2,
	};
	RootNodeTreatment RootTreatment = Put_up_with_it;
};

PulledScene PullScene(const aiScene *scene, PullOptions options = {});

struct PullException : std::exception
{
	inline virtual const char *what() const noexcept override
	{
		return "Failed to complete an unknown importing operation";
	}
};