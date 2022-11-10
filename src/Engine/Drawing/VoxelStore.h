/**
* \file: VoxelStore.h
* 
* VoxelStore.h contains classes necessary for loading and interpretting Voxel Block data.
* The types of operations it does is:
* - Load Block information from files
* - Load Pre-Stitched Atlasses 
* - Load Un-Stitched Atlasses
* - Stitch Atlasses
* - Calculate Appropriate Texture Coords for Stitched Blocks
* 
* Stitching an Atlas involves conglomerating textures onto a bigger one.
* This is done for speed, less state changes (texture switching) means quicker render.
* This is done as much as possible using GL_TEXTURE_2D_ARRAY, splitting up the atlas among multiple layers.
* Theoretically to maintain perfect wrapping, one would put each face/texture into it's own layer, however this seems like it could very quickly approach the maximum number of layers allowed (GL_MAX_ARRAY_TEXTURE_LAYERS),
* As such, a predefined number of blocks/faces will be put into each texture (ideally a square number to maximize texture space)
* Something like 6 blocks per 2D layer would be good as 6 blocks * 6 faces is a square number, with 256 layers, this allows 1,536 blocks
* -
* Then each block that has been put onto an atlas, needs to have it's texture coordinates stored.
* Other code must be responsible for using the generated atlasses and putting the texture coordinates into vertices.
* 
* 
* 
*/

#pragma once

#include <array>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "Math/floaty.h"

#include "Drawing/Texture.h"
#include "Drawing/Image2DArray.h"

#include "Game/VoxelStuff/VoxelCube.h"
#include "Game/VoxelStuff/VoxelTypes.h"

#include "AtlasTypes.h"

namespace Voxel
{
	// Forward declare VoxelChunk to use as pointer
	class VoxelChunk;

	// The pixel size of a single block face
	// All face textures are assumed to be this size
	constexpr int DefaultFaceSize = 1024;
	constexpr int DefaultAtlasLayerFaceCount = 1;

	struct FaceTexCoord
	{
		floaty3 LowerTexCoord;
		floaty3 UpperTexCoord;
	};

	struct VoxelBlockMesh
	{
		std::array<std::vector<VoxelVertex>, 6> FaceVertices;
		std::array<std::vector<unsigned int>, 6> FaceIndices;
	};

	struct VoxelBlock
	{
		std::string Name;
		std::string AtlasName;
		SerialBlock BlockData;

		// VerticesID stores an ID into shared mesh geometry
		// Vertices contains a copy of the shared mesh geometry with correct texture coords
		std::string MeshName;
		VoxelBlockMesh Mesh;

		// Used as key into Atlas
		std::array<std::string, 6> DiffuseTexNames;

		std::array<FaceClosedNess, 6> FaceOpaqueness;
		bool WantsUpdate;
	};

	struct StitchedAtlasSet
	{
		std::string AtlasName;

		std::shared_ptr<Drawing::Image2DArray> DiffuseImage;
		std::shared_ptr<Drawing::Image2DArray> SpecularImage;
		std::shared_ptr<Drawing::Image2DArray> EmissiveImage;
		std::shared_ptr<Drawing::Image2DArray> NormalImage;
		std::shared_ptr<Drawing::Image2DArray> BumpImage;
		
		constexpr std::shared_ptr<Drawing::Image2DArray>& GetImageFromType(AtlasType type)
		{
			switch (type)
			{
			default:
			case AtlasType::DIFFUSE:
				return DiffuseImage;
			case AtlasType::SPECULAR:
				return SpecularImage;
			case AtlasType::EMISSIVE:
				return EmissiveImage;
			case AtlasType::NORMAL:
				return NormalImage;
			case AtlasType::BUMP:
				return BumpImage;
			}
		}

		// Use to lookup where a particular set of images are stored within the atlas.
		// 
		// The std::array<float, 5> returned contains: ([0]: u-min, [1]: v-min, [2]: u-max, [3]: v-max, [4]: texture layer (usually the 3rd component of a texture2darray lookup))
		// An example Atlas that stores 1 image per layer, with only 1 image, would look like:
		// { "texture1": { (u-min): 0.f, (v-min): 0.f, (u-max): 1.f, (v-max): 1.f, (layer): 0.f} }
		// An example Atlas that stores 4 images (in a 2x2) per layer, with 5 images would contain:
		// { "texture1": { 0.f, 0.f, 0.5f, 0.5f, 0.f },
		//   "texture2": { 0.5f, 0.f, 1.f, 0.5f, 0.f },
		//   "texture3": { 0.f, 0.5f, 0.5f, 1.f, 0.f },
		//   "texture4": { 0.5f, 0.5f, 1.f, 1.f, 0.f },
		//   "texture5": { 0.f, 0.f, 0.5f, 0.5f, 1.f }  }
		std::unordered_map<std::string, std::array<float, 5>> FaceTextureLookup;

		floaty3 ConvertTexCoords(const std::string& diffuseTexName, floaty2 uvIn) const;
		inline bool ContainsMappingFor(std::string diffuseTexName) const { return FaceTextureLookup.count(diffuseTexName); }
	};

	struct BlockDescription
	{
		std::string Name;
		std::string AtlasName;
		SerialBlock Data;
		std::array<FaceClosedNess, 6> FaceOpaqueness;
		bool WantsUpdate;
		std::string MeshName;

		std::array<std::string, 6> DiffuseFaceTextures;
		std::array<std::string, 6> NormalFaceTextures;
		std::array<std::string, 6> BumpFaceTextures;
		std::array<std::string, 6> SpecularFaceTextures;
		std::array<std::string, 6> EmissiveFaceTextures;
		
		inline constexpr const std::array<std::string, 6>& GetFacesFor(AtlasType type) const
		{
			switch (type)
			{
			default:
			case AtlasType::DIFFUSE:
				return DiffuseFaceTextures;
			case AtlasType::NORMAL:
				return NormalFaceTextures;
			case AtlasType::BUMP:
				return BumpFaceTextures;
			case AtlasType::SPECULAR:
				return SpecularFaceTextures;
			case AtlasType::EMISSIVE:
				return EmissiveFaceTextures;
			}
		}
	};

	BlockDescription GetEmptyBlockDesc();

	struct UnStitchedAtlasSet
	{
		// Prefix given to each sub-atlas (sub-atlasses being diffuse/normal/specular/emissive/bump)
		std::string AtlasPrefix;
		std::vector<BlockDescription> Blocks;
	};

	//Temporarily not caring about prestitched
	struct PreStitchedAtlas
	{
		std::string AtlasGroup;
		std::string Filename;
		std::vector<VoxelBlock> Blocks;
	};

	// Used for in between stitching
	struct MidStitchData
	{
		int curLayer;
		int curX;
		int curY;
	};

	class VoxelStore
	{
		static std::unique_ptr<VoxelStore> _instance;

	public:

		class Accessor
		{
		public:
			inline const VoxelStore& operator()() const { return *_instance; }
			inline operator const VoxelStore& () const { return *_instance; }
		} static Instance;

		static const VoxelBlock EmptyBlock;
		static const BlockDescription EmptyBlockDesc;
		static const std::string DefaultCubeMeshName;

		static VoxelStore& GetMutable();

	private:

		std::vector<std::shared_ptr<StitchedAtlasSet>> _sets;
		std::unordered_map<std::string, std::shared_ptr<StitchedAtlasSet>> _atlasLookup;
		std::unordered_map<std::string, size_t> _descriptionLookup;
		std::unordered_map<std::string, VoxelBlockMesh> _voxelMeshLookup;
		std::unordered_map<std::string, std::unique_ptr<ICube>> _voxelUpdateBlockLookup;
		std::vector<VoxelBlock> _descriptions;
		std::vector<BlockDescription> _unstitchedDescriptions;

		void LoadAtlas(const std::string& path);
		void LoadBlockFile(const std::string& path);
		void LoadAtlasDirectory(const std::string& directory);
		void LoadBlockDirectory(const std::string& directory);
		void LoadMeshFromFile(const std::string& path);
		void LoadMeshesFromDirectory(const std::string& path);
		void LoadDefaultCube();

		void StitchUnstitched(const std::string& faceTexDir);

		void CheckForNoAtlasses() const;

		VoxelBlock& GetOrCreateBlock(const std::string& blockName);
	public:
		/**
		* Instantiates, loads and stitches voxel atlases from disk.
		* Loads Pre-Stitched Atlases from prestitchedDirectory, (currently not implemented)
		* Loads Block Descriptions (see example_block_file.yaml) from blockDirectory,
		* When stitching atlases, the texture files described in Block Descriptions are assumed to be stored in faceTexDir (faceTexDir is pre-pended to non-absolute texture filenames)
		* Also Stitches any atlases in builtInAtlases. (Uses faceTexDir for textures)
		*/
		VoxelStore(const std::string& prestitchedDirectory, const std::string& blockDirectory, const std::string& faceTexDir, const std::string& meshDir, std::vector<UnStitchedAtlasSet> builtInAtlases = std::vector<UnStitchedAtlasSet>());


		// Loading stuff
		// v
		void StitchAndLoadAtlas(const UnStitchedAtlasSet& set, const std::string& faceTexDir);
		StitchedAtlasSet StichAtlas(const UnStitchedAtlasSet& set, const std::string& faceTexDir);

		/**
		* Partially Loads a BlockDescription into a VoxelBlock.
		* 
		* Does not load the actual atlas information, this is loaded when loading a stitched atlas.
		*/
		void LoadBlock(const BlockDescription& desc);

		bool TryGetAtlas(const std::string& name, std::shared_ptr<StitchedAtlasSet>& out) const;
		bool TryGetAtlasTexture(const AtlasTextureName& name, std::shared_ptr<Drawing::Image2DArray>& out) const;
		// ^
		// Loading stuff
		
		// Block lookup
		// v
		bool HasBlock(const std::string& name) const;
		const VoxelBlock* GetBlock(const std::string& name) const; // Return value should not be kept
		bool TryGetDescription(const std::string& name, VoxelBlock& desc) const;
		bool TryGetDescription(size_t ID, VoxelBlock& desc) const;
		bool TryGetDescription(const std::string& ID, const VoxelBlock*& desc) const;
		bool TryGetDescription(size_t ID, const VoxelBlock*& desc) const;
		const VoxelBlock* GetDescOrEmpty(size_t ID) const; // Returns either the description for the ID, or the description for 'empty' if not possible
		const VoxelBlock* GetDescOrEmpty(const std::string& name) const;
		size_t GetIDFor(const std::string& blockName) const;
		const std::string& GetNameOf(size_t ID) const;
		// ^
		// Block Lookup

		// Mesh stuff
		// v
		const VoxelBlockMesh* GetBlockVertices(const std::string& name) const;

		floaty3 ConvertToAtlasTexCoords(const std::string& atlasName, const std::string& diffuseTexName, floaty2 uv) const;
		// ^ 
		// Mesh stuff
		// Update Block stuff 
		// v
		std::unique_ptr<ICube> CreateCube(VoxelWorld* world, VoxelChunk* chunk, ChunkBlockCoord pos, const SerialBlock& blockData) const;
		std::unique_ptr<ICube> CreateCube(VoxelWorld* world, VoxelChunk* chunk, ChunkBlockCoord pos, const std::string& blockName) const;

		void RegisterUpdateBlock(const std::string& blockName, std::unique_ptr<ICube> cube);
		// ^
		// Update Block stuff


		static void InitializeVoxelStore(const std::string& prestitchedDir, const std::string& blockDir, const std::string& faceTexDir, const std::string& meshDir, std::vector<UnStitchedAtlasSet> builtInAtlases = std::vector<UnStitchedAtlasSet>());

		static const SerialBlock EmptyBlockData;
	};
}