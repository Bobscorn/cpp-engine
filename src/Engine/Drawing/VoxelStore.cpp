#include "VoxelStore.h"

#include <filesystem>
#include <algorithm>

#pragma warning(push)
#pragma warning(disable:4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning(pop)

#include "Helpers/StringHelper.h"

#include "Systems/Execution/Engine.h"
#include "Systems/Importing/SimpleMeshImport.h"

#include "Game/VoxelStuff/VoxelTypes.h"

const Voxel::SerialBlock Voxel::VoxelStore::EmptyBlockData = Voxel::SerialBlock{};

/// <summary>
/// LOADING BLOCKS:
/// Information required:
/// - Mesh
/// - Textures
/// - WantsUpdate
/// - Name
/// - AtlasName
/// 3 Stages:
/// - Load Blocks
/// - Load Meshes
/// - Load Atlas textures
/// - Replace Block Vertices with Atlas-ed Mesh vertices
/// 
/// </summary>

namespace Voxel
{
	std::unique_ptr<VoxelStore> VoxelStore::_instance{};
	VoxelStore::Accessor VoxelStore::Instance{};

	const VoxelBlock VoxelStore::EmptyBlock = VoxelBlock{ };
	const BlockDescription VoxelStore::EmptyBlockDesc = BlockDescription{};
	const std::string VoxelStore::DefaultCubeMeshName = "default-cube";

	std::vector<VoxelVertex> ConvertToVoxelVertices(const std::vector<FullVertex>& vertices)
	{
		std::vector<VoxelVertex> convertedVertices;
		convertedVertices.reserve(vertices.size());

		for (size_t i = 0; i < vertices.size(); ++i)
		{
			auto& inVert = vertices[i];
			VoxelVertex vert;
			vert.Position = inVert.PosL;
			vert.Normal = inVert.NormalL;
			vert.Binormal = inVert.BinormalL;
			vert.Tangent = inVert.TangentL;
			vert.TexCoord = floaty3{ inVert.Tex.x, inVert.Tex.y, 0.f };
			convertedVertices.push_back(vert);
		}

		return convertedVertices;
	}

	void VoxelStore::LoadAtlas(const std::string& path)
	{
		if (std::filesystem::exists(path))
		{
			try
			{
				DINFO("Loading Atlas file '" + path + "' ");

				auto node = YAML::LoadFile(path);

				DWARNING("Loading atlas files not currently implemented!");
			}
			catch (YAML::Exception& e)
			{
				DERROR("Failed to load Atlas file '" + path + "': " + e.what());
			}
		}
	}

	void VoxelStore::LoadBlockFile(const std::string& path)
	{
		/**
		* Loads a YAML file into one or more BlockDescription(s).
		* Expects a 'block' tag at the root that is a sequence.
		* Each item in the sequence must contain:
		* - block-name: [string block id]
		* - atlas: [name of atlas containing textures]
		* - wants-update: [true/false/1/0] (describes whether to instantiate an ICube from this block rather then leave as a SerialBlock
		* EITHER:
		* - faces: [opaque/semi-opaque/transparent]
		* OR:
		* - faces:
		*   - pos-x: [opaque/semi-opaque/transparent]
		*   - neg-x: [opaque/semi-opaque/transparent]
		*   - pos-y: [opaque/semi-opaque/transparent]
		*   - neg-y: [opaque/semi-opaque/transparent]
		*   - pos-z: [opaque/semi-opaque/transparent]
		*   - neg-z: [opaque/semi-opaque/transparent]
		* EITHER:
		* - textures: elsewhere (not currently implemented, this mode implies you will load the textures for this block in another way)
		* OR:
		* - textures:
		*   - pos-x: [texture file name]
		*   - neg-x: [texture file name]
		*   - pos-y: [texture file name]
		*   - neg-y: [texture file name]
		*   - pos-z: [texture file name]
		*   - neg-z: [texture file name]
		*/
		if (std::filesystem::exists(path))
		{
			try
			{
				DINFO("Loading Block file '" + path + "'");

				auto doc = YAML::LoadFile(path);

				auto blocks = doc["blocks"];
				if (!blocks)
				{
					DINFO("Block file '" + path + "' is empty!");
					return;
				}

				if (!blocks.IsSequence())
				{
					DWARNING("Block file '" + path + "' does not contain a sequence under the blocks tag!");
					return;
				}

				for (auto it = blocks.begin(); it != blocks.end(); ++it)
				{
					auto node = *it;
					
					BlockDescription desc;

					auto name = node["block-name"];
					if (!name || !name.IsScalar())
					{
						DWARNING("Skipping sequence item without or with invalid 'block-name' tag");
						continue;
					}

					desc.Name = name.Scalar();

					auto atlasNode = node["atlas"];
					if (!atlasNode || !atlasNode.IsScalar())
					{
						DWARNING("Skipping block with no or invalid 'atlas' tag (currently all blocks are required to have an atlas tag, this may change when pre-stitched atlases are added)");
						continue;
					}

					desc.AtlasName = atlasNode.Scalar();

					// SerialBlock loading:
					desc.Data.ID = 0;
					desc.Data.Data.Rotation = quat4::identity();
					desc.WantsUpdate = false;

					auto wantsUpdateNode = node["wants-update"];
					if (wantsUpdateNode)
					{
						if (!wantsUpdateNode.IsScalar())
						{
							DWARNING("Block '" + desc.Name + "' has invalid 'wants-update' tag! If you supply the wants-update tag it must be a scalar! (Just a string!)");
						}
						else
						{
							auto& val = wantsUpdateNode.Scalar();
							if (!StringHelper::IfBool(val, &desc.WantsUpdate))
							{
								DWARNING("Block '" + desc.Name + "' has invalid 'wants-update' value of '" + val + "' expected a boolean value!");
							}
						}
					}

					auto facesNode = node["faces"];
					if (!facesNode)
					{
						DWARNING("Block '" + desc.Name + "' has no 'faces' tag! You must specify the opacity of all block's faces!");
						continue;
					}
					if (facesNode.IsScalar())
					{
						auto setFaceFunc = [](FaceClosedNess& faceNess, FaceClosedNess val) { faceNess = val; };
						auto setAllFacesFunc = [&desc, &setFaceFunc](FaceClosedNess opaqueNess) { std::for_each(desc.FaceOpaqueness.begin(), desc.FaceOpaqueness.end(), std::bind(setFaceFunc, std::placeholders::_1, opaqueNess));  };
						auto& val = facesNode.Scalar();
						if (val == "closed")
						{
							setAllFacesFunc(FaceClosedNess::CLOSED_FACE);
						}
						else if (val == "semi-closed" || val == "semi-open")
						{
							setAllFacesFunc(FaceClosedNess::SEMI_CLOSED_FACE);
						}
						else if (val == "open")
						{
							setAllFacesFunc(FaceClosedNess::OPEN_FACE);
						}
						else
						{
							DWARNING("Block '" + desc.Name + "' has an invalid 'faces' tag value of '" + val + "'! It must either be 'closed', 'semi-closed', 'semi-open', 'open' or a map specifying this value per face!(via pos - x: neg - x : etc.children)");
							continue;
						}
					}
					else if (facesNode.IsMap())
					{
						auto setFaceFromNodeFunc = [&desc](const std::string& name, const YAML::Node& node, FaceClosedNess& face)
						{
							if (!node)
							{
								DWARNING("Block '" + desc.Name + "' has no '" + name + "' tag! (As a child of the faces tag)");
								return false;
							}
							if (!node.IsScalar())
							{
								DWARNING("Block '" + desc.Name + "' has an invalid '" + name + "' tag! (As a child of the faces tag)");
								return false;
							}
							auto& val = node.Scalar();
							if (val == "closed")
								face = FaceClosedNess::CLOSED_FACE;
							else if (val == "semi-closed" || val == "semi-open")
								face = FaceClosedNess::SEMI_CLOSED_FACE;
							else if (val == "open")
								face = FaceClosedNess::OPEN_FACE;
							else
							{
								DWARNING("Block '" + desc.Name + "' has tag '" + name + "' (child of faces tag) has unknown value of '" + val + "'!");
								return false;
							}

							return true;
						};

						if (!setFaceFromNodeFunc("pos-x", facesNode["pos-x"], desc.FaceOpaqueness[(size_t)BlockFace::POS_X]))
							continue;
						if (!setFaceFromNodeFunc("neg-x", facesNode["neg-x"], desc.FaceOpaqueness[(size_t)BlockFace::NEG_X]))
							continue;
						if (!setFaceFromNodeFunc("pos-y", facesNode["pos-y"], desc.FaceOpaqueness[(size_t)BlockFace::POS_Y]))
							continue;
						if (!setFaceFromNodeFunc("neg-y", facesNode["neg-y"], desc.FaceOpaqueness[(size_t)BlockFace::NEG_Y]))
							continue;
						if (!setFaceFromNodeFunc("pos-z", facesNode["pos-z"], desc.FaceOpaqueness[(size_t)BlockFace::POS_Z]))
							continue;
						if (!setFaceFromNodeFunc("neg-z", facesNode["neg-z"], desc.FaceOpaqueness[(size_t)BlockFace::NEG_Z]))
							continue;
					}
					else
					{
						DWARNING("Block '" + desc.Name + "' has invalid faces tag!");
						continue;
					}


					// ^
					// SerialBlock loading
					// Textures
					// v

					auto tex = node["textures"];
					if (!tex || (tex.IsScalar() && tex.Scalar() == "elsewhere"))
					{
						desc.AtlasName = "";
						std::fill(desc.DiffuseFaceTextures.begin(),  desc.DiffuseFaceTextures.end(),  std::string{});
						std::fill(desc.NormalFaceTextures.begin(),	 desc.NormalFaceTextures.end(),   std::string{});
						std::fill(desc.BumpFaceTextures.begin(),	 desc.BumpFaceTextures.end(),	  std::string{});
						std::fill(desc.SpecularFaceTextures.begin(), desc.SpecularFaceTextures.end(), std::string{});
						std::fill(desc.EmissiveFaceTextures.begin(), desc.EmissiveFaceTextures.end(), std::string{});
					}
					else if (tex.IsMap())
					{
						auto& blockName = desc.Name;
						auto getTextureGroup = [&blockName](const YAML::Node& groupNode, std::array<std::string, 6>& faces)
						{
							if (groupNode && groupNode.IsMap())
							{
								auto getFaceFunc = [&blockName](const std::string& name, const YAML::Node& n, std::string& texFileRef)
								{
									if (!n)
									{
										DINFO("Block '" + blockName + "' missing expected tag '" + name + "' (expected because textures tag is not 'elsewhere' and is a YAML map)");
										return;
									}
									if (!n.IsScalar())
									{
										DINFO("Block '" + blockName + "' contains invalid '" + name + "' tag!");
										return;
									}
									texFileRef = n.Scalar();
								};

								getFaceFunc("pos-x", groupNode["pos-x"], faces[(size_t)BlockFace::POS_X]);
								getFaceFunc("pos-y", groupNode["pos-y"], faces[(size_t)BlockFace::POS_Y]);
								getFaceFunc("pos-z", groupNode["pos-z"], faces[(size_t)BlockFace::POS_Z]);
								getFaceFunc("neg-x", groupNode["neg-x"], faces[(size_t)BlockFace::NEG_X]);
								getFaceFunc("neg-y", groupNode["neg-y"], faces[(size_t)BlockFace::NEG_Y]);
								getFaceFunc("neg-z", groupNode["neg-z"], faces[(size_t)BlockFace::NEG_Z]);
							}
						};
						
						getTextureGroup(tex["diffuse"], desc.DiffuseFaceTextures);
						getTextureGroup(tex["normal"], desc.NormalFaceTextures);
						getTextureGroup(tex["specular"], desc.SpecularFaceTextures);
						getTextureGroup(tex["bump"], desc.BumpFaceTextures);
						getTextureGroup(tex["emissive"], desc.EmissiveFaceTextures);

						if (std::any_of(desc.DiffuseFaceTextures.begin(), desc.DiffuseFaceTextures.end(), [](const std::string& s) { return s.size(); }))
							_unstitchedDescriptions.emplace_back(desc);
					}
					else
					{
						DINFO("Block '" + desc.Name + "' in block file '" + path + "' contains an invalid textures tag");
						continue;
					}

					auto meshNode = node["mesh"];
					desc.MeshName = DefaultCubeMeshName;
					if (meshNode)
					{
						if (!meshNode.IsScalar())
						{
							DWARNING("Block '" + desc.Name + "' has invalid mesh tag!");
							DWARNING("The mesh tag should just be a simple value! (like mesh: super_cube.dae");
						}
						else
						{
							desc.MeshName = meshNode.Scalar();
						}
					}

					LoadBlock(desc);
				}
			}
			catch (YAML::Exception& e)
			{
				DERROR("Exception when loading block file '" + path + "': " + e.what());
			}
		}
	}

	void VoxelStore::LoadAtlasDirectory(const std::string& directory)
	{
		if (std::filesystem::exists(directory))
		{
			if (std::filesystem::is_directory(directory))
			{
				for (auto& item : std::filesystem::directory_iterator(directory))
				{
					LoadAtlas(item.path().string());
				}
			}
			else
			{
				DERROR("Failed to load atlas directory: '" + directory + "' is not a valid directory");
			}
		}
		else
		{
			DINFO("Atlas directory does not exist");
		}
	}

	void VoxelStore::LoadBlockDirectory(const std::string& directory)
	{
		if (std::filesystem::exists(directory))
		{
			if (std::filesystem::is_directory(directory))
			{
				for (auto& item : std::filesystem::directory_iterator(directory))
				{
					LoadBlockFile(item.path().string());
				}
			}
			else
			{
				DERROR("Failed to load block directory: '" + directory + "' is not a valid directory");
			}
		}
		else
		{
			DINFO("Block directory does not exist");
		}
	}

	void VoxelStore::LoadMeshFromFile(const std::string& path)
	{
		auto simpleMeshes = Importing::ImportAllSimpleMeshes(path);

		if (!simpleMeshes.size())
		{
			DWARNING("Could not load meshes from file '" + path + "'");
			return;
		}

		VoxelBlockMesh voxelVerts;

		const static std::array<std::string, 6> blockFaceNames = { "pos-y", "neg-z", "pos-x", "neg-y", "pos-z", "neg-x" };
		std::vector<int> remainingBlockFaceIndices = { 0, 1, 2, 3, 4, 5 };
		for (auto& mesh : simpleMeshes)
		{
			if (remainingBlockFaceIndices.empty())
				break;
			for (int i = 0; i < remainingBlockFaceIndices.size(); ++i)
			{
				auto& blockFaceIndex = remainingBlockFaceIndices[i];
				if (mesh.MeshName.rfind(blockFaceNames[blockFaceIndex]) == 0)
				{
					voxelVerts.FaceVertices[blockFaceIndex] = ConvertToVoxelVertices(mesh.Vertices);
					voxelVerts.FaceIndices[blockFaceIndex] = mesh.Indices;

					std::swap(remainingBlockFaceIndices[i], remainingBlockFaceIndices.back());
					remainingBlockFaceIndices.pop_back();
					break;
				}
			}
		}

		if (remainingBlockFaceIndices.size())
		{
			DWARNING("Mesh in file '" + path + "' does not contain 6 meshes of names 'pos-y', 'neg-z', 'pos-x', 'neg-y', 'pos-z' and 'neg-x'!");
			return;
		}

		auto p = std::filesystem::path(path);
		_voxelMeshLookup[p.stem().string()] = std::move(voxelVerts);
	}

	void VoxelStore::LoadMeshesFromDirectory(const std::string& path)
	{
		if (std::filesystem::exists(path))
		{
			if (std::filesystem::is_directory(path))
			{
				for (auto& item : std::filesystem::directory_iterator(path))
				{
					LoadMeshFromFile(item.path().string());
				}
			}
			else
			{
				DWARNING("Given path '" + path + "' is not a directory!");
			}
		}
		else
		{
			DWARNING("Given path '" + path + "' does not exist!");
		}
	}

	void VoxelStore::LoadDefaultCube()
	{
		auto& mesh = _voxelMeshLookup[DefaultCubeMeshName];

		auto addFaceVerts = [&mesh](BlockFace face)
		{
			auto direction = BlockFaceHelper::GetDirection(face);

			auto tangent = BlockFaceHelper::GetTangentDirection(face);

			auto binorm = direction.cross(tangent);
			auto& vertices = mesh.FaceVertices[(size_t)face];
			auto& indices = mesh.FaceIndices[(size_t)face];

			// Generative base vertex for copying
			Voxel::VoxelVertex vert1{};
			vert1.Position = direction * 0.5f * BlockSize + tangent * 0.5f * BlockSize + binorm * 0.5f * BlockSize;
			vert1.Normal = direction;
			vert1.Tangent = tangent;
			vert1.Binormal = binorm;
			vert1.TexCoord = { 1.f, 1.f, 0.f };
			Voxel::VoxelVertex vert2 = vert1;
			vert2.Position = direction * 0.5f * BlockSize + tangent * -0.5f * BlockSize + binorm * 0.5f * BlockSize;
			vert2.TexCoord = { 0.f, 1.f, 0.f };
			Voxel::VoxelVertex vert3 = vert1;
			vert3.Position = direction * 0.5f * BlockSize + tangent * -0.5f * BlockSize + binorm * -0.5f * BlockSize;
			vert3.TexCoord = { 0.f, 0.f, 0.f };
			Voxel::VoxelVertex vert4 = vert1;
			vert4.Position = direction * 0.5f * BlockSize + tangent * 0.5f * BlockSize + binorm * -0.5f * BlockSize;
			vert4.TexCoord = { 1.f, 0.f, 0.f };
			GLuint index1 = 0;
			GLuint index2 = 1;
			GLuint index3 = 2;
			GLuint index4 = 2;
			GLuint index5 = 3;
			GLuint index6 = 0;
			GLuint indexOffset = (GLuint)vertices.size();
			// insert indices
			indices.emplace_back(index1 + indexOffset);
			indices.emplace_back(index2 + indexOffset);
			indices.emplace_back(index3 + indexOffset);
			indices.emplace_back(index4 + indexOffset);
			indices.emplace_back(index5 + indexOffset);
			indices.emplace_back(index6 + indexOffset);
			// insert vertices
			vertices.emplace_back(vert1);
			vertices.emplace_back(vert2);
			vertices.emplace_back(vert3);
			vertices.emplace_back(vert4);
		};

		for (auto& face : BlockFacesArray)
		{
			addFaceVerts(face);
		}
	}

	void VoxelStore::StitchUnstitched(const std::string& faceTexDir)
	{
		// Currently this must be called once and must be called upon creation
		if (_unstitchedDescriptions.size())
		{
			UnStitchedAtlasSet unstitchedDiffuse{ "global", std::move(_unstitchedDescriptions) };
			StitchAndLoadAtlas(unstitchedDiffuse, faceTexDir);
		}
	}

	void VoxelStore::CheckForNoAtlasses() const
	{
		for (auto& block : _descriptions)
		{
			if (block.AtlasName.empty())
			{
				DINFO("Block '" + block.Name + "' has no atlas!");
			}
		}
	}

	VoxelBlock& VoxelStore::GetOrCreateBlock(const std::string& blockName)
	{
		auto it = _descriptionLookup.find(blockName);
		if (it == _descriptionLookup.end() || it->second >= _descriptions.size())
		{
			_descriptionLookup[blockName] = _descriptions.size();
			_descriptions.emplace_back();
			return _descriptions.back();
		}
		return _descriptions[it->second];
	}

	VoxelStore::VoxelStore(const std::string& prestitchedDirectory, const std::string& blockDirectory, const std::string& faceTexDir, const std::string& meshDir, std::vector<UnStitchedAtlasSet> builtInAtlases)
	{
		LoadBlock(GetEmptyBlockDesc()); // Load empty block description first
		LoadMeshesFromDirectory(meshDir);
		LoadDefaultCube();
		LoadBlockDirectory(blockDirectory);
		(void)prestitchedDirectory;//LoadAtlasDirectory(prestitchedDirectory); No! Not Yet My Boi! (not implemented)

		StitchUnstitched(faceTexDir);

		for (int i = 0; i < builtInAtlases.size(); ++i)
		{
			StitchAndLoadAtlas(builtInAtlases[i], faceTexDir);
		}
	}

	void VoxelStore::StitchAndLoadAtlas(const UnStitchedAtlasSet& set, const std::string& faceTexDir)
	{
		if (set.Blocks.empty())
			return;

		{
			auto tmp = std::shared_ptr<StitchedAtlasSet>();
			if (TryGetAtlas(set.AtlasPrefix, tmp))
			{
				DWARNING("Overwriting Existing atlas '" + set.AtlasPrefix + "'!");
			}
		}

		auto ptr = std::make_shared<StitchedAtlasSet>(StichAtlas(set, faceTexDir));

		_atlasLookup[set.AtlasPrefix] = ptr;

		for (auto& desc : _descriptions)
		{
			if (desc.AtlasName == ptr->AtlasName)
			{
				for (int faceI = 0; faceI < 6; ++faceI)
				{
					if (!ptr->ContainsMappingFor(desc.DiffuseTexNames[faceI]))
						continue;

					for (int i = 0; i < desc.Mesh.FaceVertices[faceI].size(); ++i)
					{
						auto& vert = desc.Mesh.FaceVertices[faceI][i];
						vert.TexCoord = ptr->ConvertTexCoords(desc.DiffuseTexNames[faceI], (floaty2)vert.TexCoord);
					}
				}
			}
		}
	}

	StitchedAtlasSet VoxelStore::StichAtlas(const UnStitchedAtlasSet& set, const std::string& faceTexDir)
	{
		if (set.Blocks.empty())
			return StitchedAtlasSet{ set.AtlasPrefix, nullptr, nullptr, nullptr, nullptr, nullptr, std::unordered_map<std::string, std::array<float, 5>>{} };

		MidStitchData data{};
		int faceSize = DefaultFaceSize;
		float faceSizef = (float)DefaultFaceSize;

		float atlasLayerSize = (float)faceSize * DefaultAtlasLayerFaceCount + (DefaultAtlasLayerFaceCount - 1) * 2;
		int faceCount = DefaultAtlasLayerFaceCount;

		StitchedAtlasSet stitched;
		stitched.AtlasName = set.AtlasPrefix;
		stitched.DiffuseImage = std::make_shared<Drawing::Image2DArray>( (size_t)faceCount * faceSize + (faceCount - 1) * 2, (size_t)faceCount * faceSize + (faceCount - 1) * 2, 1 );
		stitched.SpecularImage = std::make_shared<Drawing::Image2DArray>( (size_t)faceCount * faceSize + (faceCount - 1) * 2, (size_t)faceCount * faceSize + (faceCount - 1) * 2, 1 );
		stitched.EmissiveImage = std::make_shared<Drawing::Image2DArray>( (size_t)faceCount * faceSize + (faceCount - 1) * 2, (size_t)faceCount * faceSize + (faceCount - 1) * 2, 1 );
		stitched.NormalImage = std::make_shared<Drawing::Image2DArray>( (size_t)faceCount * faceSize + (faceCount - 1) * 2, (size_t)faceCount * faceSize + (faceCount - 1) * 2, 1 );
		stitched.BumpImage = std::make_shared<Drawing::Image2DArray>( (size_t)faceCount * faceSize + (faceCount - 1) * 2, (size_t)faceCount * faceSize + (faceCount - 1) * 2, 1 );
		stitched.FaceTextureLookup = std::unordered_map<std::string, std::array<float, 5>>();

		auto& dif = stitched.DiffuseImage;
		auto& norm = stitched.NormalImage;
		auto& bump = stitched.BumpImage;
		auto& spec = stitched.SpecularImage;
		auto& emis = stitched.EmissiveImage;

		VoxelBlock block;
		block.AtlasName = set.AtlasPrefix;

		auto getSurfaceFromFile = [&faceSize, &faceTexDir](const std::string& fileName)
		{
			if (fileName == "empty" || fileName == "")
			{
				SDL_Surface* p = SDL_CreateRGBSurfaceWithFormat(0, faceSize, faceSize, 32, SDL_PIXELFORMAT_RGBA32);
				SDL_FillRect(p, nullptr, SDL_MapRGBA(p->format, 0, 0, 0, 0xff));
				return SimpleSurface(p);
			}
			std::filesystem::path p{ fileName };
			if (p.is_relative())
				p = faceTexDir / p;
			return SimpleSurface(Drawing::SDLImage::LoadSurface(p.string()));
		};

		auto addFaceFunc = [&data, &faceSize, &faceSizef, &faceCount, &atlasLayerSize, &faceTexDir, &dif, &norm, &bump, &spec, &emis, &getSurfaceFromFile]
		(std::array<float, 5>& face,
			const std::string& difFaceFile,
			const std::string& specFaceFile,
			const std::string& emisFaceFile,
			const std::string& normFaceFile,
			const std::string& bumpFaceFile,
			bool exists)
		{
			SimpleSurface difFaceSurf = (exists ? nullptr : getSurfaceFromFile(difFaceFile));
			SimpleSurface specFaceSurf = getSurfaceFromFile(specFaceFile);
			SimpleSurface emisFaceSurf = getSurfaceFromFile(emisFaceFile);
			SimpleSurface normFaceSurf = getSurfaceFromFile(normFaceFile);
			SimpleSurface bumpFaceSurf = getSurfaceFromFile(bumpFaceFile);
			if (!difFaceSurf)
				return;
			SDL_Rect dst;
			dst.x = data.curX * faceSize + data.curX * 2;
			dst.y = data.curY * faceSize + data.curY * 2;
			dst.w = faceSize;
			dst.h = faceSize;

			auto doFaceThing = [&dst, &data, &faceSize, &faceCount](const std::shared_ptr<Drawing::Image2DArray>& img, const SimpleSurface& surface)
			{
				if (!surface)
					return;

				img->SetArea(surface.Get(), dst, data.curLayer);

				if (data.curX > 0)
					img->SetArea(surface.Get(), SDL_Rect{ 0, 0, 1, faceSize }, SDL_Rect{ dst.x - 1, dst.y, 1, faceSize }, data.curLayer);
				if (data.curY > 0)
					img->SetArea(surface.Get(), SDL_Rect{ 0, 0, faceSize, 1 }, SDL_Rect{ dst.x, dst.y - 1, faceSize, 1 }, data.curLayer);
				if (data.curX < faceCount - 1)
					img->SetArea(surface.Get(), SDL_Rect{ faceSize - 1, 0, 1, faceSize }, SDL_Rect{ dst.x + faceSize, dst.y, 1, faceSize }, data.curLayer);
				if (data.curY < faceCount - 1)
					img->SetArea(surface.Get(), SDL_Rect{ 0, faceSize - 1, faceSize, 1 }, SDL_Rect{ dst.x, dst.y + faceSize, faceSize, 1 }, data.curLayer);
			};

			doFaceThing(dif, difFaceSurf);
			doFaceThing(norm, normFaceSurf);
			doFaceThing(bump, bumpFaceSurf);
			doFaceThing(spec, specFaceSurf);
			doFaceThing(emis, emisFaceSurf);

			if (!exists)
			{
				face[0] = (float)dst.x / atlasLayerSize;
				face[1] = (float)dst.y / atlasLayerSize;
				face[2] = face[0] + faceSizef / atlasLayerSize;
				face[3] = face[1] + faceSizef / atlasLayerSize;
				face[4] = (float)data.curLayer;

				if (++data.curX >= faceCount)
				{
					data.curX = 0;
					if (++data.curY >= faceCount)
					{
						dif->AddLayer();
						norm->AddLayer();
						bump->AddLayer();
						spec->AddLayer();
						emis->AddLayer();
						++data.curLayer;
						data.curY = 0;
					}
				}
			}
		};
		
		for (int i = 0; i < set.Blocks.size(); ++i)
		{
			auto& desc = set.Blocks[i];

			block.Name = desc.Name;
			for (int j = 0; j < 6; ++j)
			{
				addFaceFunc(
					stitched.FaceTextureLookup[desc.DiffuseFaceTextures[j]], 
					desc.DiffuseFaceTextures[j], 
					desc.SpecularFaceTextures[j],
					desc.EmissiveFaceTextures[j], 
					desc.NormalFaceTextures[j], 
					desc.BumpFaceTextures[j],
					stitched.FaceTextureLookup.count(desc.DiffuseFaceTextures[j])
				);
			}
		}

		stitched.DiffuseImage->LoadGL();
		stitched.DiffuseImage->GenerateMipmaps();
		stitched.NormalImage->LoadGL();
		stitched.NormalImage->GenerateMipmaps();
		stitched.BumpImage->LoadGL();
		stitched.BumpImage->GenerateMipmaps();
		stitched.SpecularImage->LoadGL();
		stitched.SpecularImage->GenerateMipmaps();
		stitched.EmissiveImage->LoadGL();
		stitched.EmissiveImage->GenerateMipmaps();

		return stitched;
	}

	void VoxelStore::LoadBlock(const BlockDescription& desc)
	{
		size_t index;
		auto it = _descriptionLookup.find(desc.Name);
		if (it != _descriptionLookup.end() && it->second < _descriptions.size())
		{
			index = it->second;
		}
		else
		{
			index = _descriptions.size();
			_descriptions.emplace_back();
			_descriptionLookup[desc.Name] = index;
		}

		auto& vox = _descriptions[index];
		vox.Name = desc.Name;
		vox.BlockData = desc.Data;
		vox.BlockData.ID = index;
		vox.FaceOpaqueness = desc.FaceOpaqueness;
		vox.MeshName = desc.MeshName;
		vox.AtlasName = desc.AtlasName;
		vox.DiffuseTexNames = desc.DiffuseFaceTextures;
		
		auto* mesh = GetBlockVertices(desc.MeshName);
		if (mesh)
		{
			vox.Mesh = *mesh;
		}
		
		// Set other properties for blocks here
		// FaceTexCoords are set when loading an atlas
	}

	bool VoxelStore::TryGetAtlas(const std::string& name, std::shared_ptr<StitchedAtlasSet>& out) const
	{
		auto it = _atlasLookup.find(name);
		if (it == _atlasLookup.end())
			return false;
		out = it->second;
		return true;
	}

	bool VoxelStore::TryGetAtlasTexture(const AtlasTextureName& name, std::shared_ptr<Drawing::Image2DArray>& out) const
	{
		auto it = _atlasLookup.find(name.AtlasName);
		if (it == _atlasLookup.end())
			return false;

		out = it->second->GetImageFromType(name.AtlasType);
		return true;
	}

	bool VoxelStore::HasBlock(const std::string& name) const
	{
		auto it = _descriptionLookup.find(name);
		return it != _descriptionLookup.end() && it->second < _descriptions.size();
	}

	const VoxelBlock* VoxelStore::GetBlock(const std::string& name) const
	{
		auto lookupIt = _descriptionLookup.find(name);
		if (lookupIt == _descriptionLookup.end())
			return nullptr;

		auto id = lookupIt->second;
		if (id >= _descriptions.size())
			return nullptr;

		return &_descriptions[id];
	}

	bool VoxelStore::TryGetDescription(const std::string& name, VoxelBlock& desc) const
	{
		auto it = _descriptionLookup.find(name);
		if (it == _descriptionLookup.end())
			return false;

		return TryGetDescription(it->second, desc);
	}

	bool VoxelStore::TryGetDescription(size_t ID, VoxelBlock& desc) const
	{
		if (ID >= _descriptions.size())
			return false;

		desc = _descriptions[ID];
		return true;
	}

	bool VoxelStore::TryGetDescription(const std::string& ID, const VoxelBlock*& desc) const
	{
		auto it = _descriptionLookup.find(ID);
		if (it == _descriptionLookup.end())
			return false;

		return TryGetDescription(it->second, desc);
	}

	bool VoxelStore::TryGetDescription(size_t ID, const VoxelBlock*& desc) const
	{
		if (ID >= _descriptions.size())
			return false;

		desc = &_descriptions[ID];
		return true;
	}

	const VoxelBlock* VoxelStore::GetDescOrEmpty(size_t ID) const
	{
		const VoxelBlock* out;
		if (!TryGetDescription(ID, out))
			return &EmptyBlock;

		return out;
	}

	const VoxelBlock* VoxelStore::GetDescOrEmpty(const std::string& name) const
	{
		return GetDescOrEmpty(GetIDFor(name));
	}

	const VoxelBlockMesh* VoxelStore::GetBlockVertices(const std::string& name) const
	{
		if (auto it = _voxelMeshLookup.find(name); it != _voxelMeshLookup.end())
		{
			return &it->second;
		}
		return nullptr;
	}

	floaty3 VoxelStore::ConvertToAtlasTexCoords(const std::string& atlasName, const std::string& diffuseName, floaty2 uv) const
	{
		if (auto it = _atlasLookup.find(atlasName); it != _atlasLookup.end())
		{
			return it->second->ConvertTexCoords(diffuseName, uv);
		}
		return { 0.f, 0.f, 0.f };
	}

	size_t VoxelStore::GetIDFor(const std::string& blockName) const
	{
		auto it = _descriptionLookup.find(blockName);
		if (it != _descriptionLookup.end() && it->second < _descriptions.size())
			return it->second;

		return 0;
	}

	const std::string& VoxelStore::GetNameOf(size_t ID) const
	{
		const static std::string unknown_string = "unknown";

		if (ID >= _descriptions.size())
			return unknown_string;

		return _descriptions[ID].Name;
	}

	void VoxelStore::InitializeVoxelStore(const std::string& prestitchedDir, const std::string& blockDir, const std::string& faceTexDir, const std::string& meshDir, std::vector<UnStitchedAtlasSet> builtInAtlases)
	{
		if (_instance)
			return;

		_instance = std::make_unique<VoxelStore>(prestitchedDir, blockDir, faceTexDir, meshDir, builtInAtlases);
	}

	std::unique_ptr<ICube> VoxelStore::CreateCube(const SerialBlock& blockData) const
	{
		extern std::unique_ptr<Engine::IEngine> g_Engine;

		auto* desc = GetDescOrEmpty(blockData.ID);
		if (desc->WantsUpdate == false)
			return nullptr;

		return std::make_unique<VoxelCube>(nullptr, &g_Engine->Resources, nullptr, floaty3{ 0.f, 0.f, 0.f }, ChunkBlockCoord{}, blockData);
	}

	std::unique_ptr<ICube> VoxelStore::CreateCube(const std::string& blockName) const
	{
		return CreateCube(GetDescOrEmpty(blockName)->BlockData);
	}

	BlockDescription GetEmptyBlockDesc()
	{
		BlockDescription desc;
		desc.Name = "empty";
		desc.AtlasName = "";
		desc.Data.ID = 0;
		desc.Data.Data.Rotation = quat4::identity();
		desc.FaceOpaqueness = { FaceClosedNess::OPEN_FACE, FaceClosedNess::OPEN_FACE, FaceClosedNess::OPEN_FACE, FaceClosedNess::OPEN_FACE, FaceClosedNess::OPEN_FACE, FaceClosedNess::OPEN_FACE };
		desc.WantsUpdate = false;
		desc.DiffuseFaceTextures = { std::string(), std::string(), std::string(), std::string(), std::string(), std::string() };
		desc.NormalFaceTextures = { std::string(), std::string(), std::string(), std::string(), std::string(), std::string() };
		desc.BumpFaceTextures = { std::string(), std::string(), std::string(), std::string(), std::string(), std::string() };
		desc.SpecularFaceTextures = { std::string(), std::string(), std::string(), std::string(), std::string(), std::string() };
		desc.EmissiveFaceTextures = { std::string(), std::string(), std::string(), std::string(), std::string(), std::string() };
		return desc;
	}

	floaty3 StitchedAtlasSet::ConvertTexCoords(const std::string& diffuseName, floaty2 uvIn) const
	{
		auto it = FaceTextureLookup.find(diffuseName);
		if (it == FaceTextureLookup.end())
		{
			DWARNING("Looking up unknown diffuse texture '" + diffuseName + "'!");
			return { 0.f, 0.f, 0.f };
		}

		auto& stitchedLocation = it->second;
		floaty2 stitchedDims = { stitchedLocation[2] - stitchedLocation[0], stitchedLocation[3] - stitchedLocation[1] };
		return floaty3{ uvIn.x * stitchedDims.x + stitchedLocation[0], uvIn.y * stitchedDims.y + stitchedLocation[1], stitchedLocation[4] };
	}
}
