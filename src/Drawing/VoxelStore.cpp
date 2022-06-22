#include "VoxelStore.h"

#include <filesystem>
#include <algorithm>

#include <yaml-cpp/yaml.h>

#include "Helpers/StringHelper.h"

#include "Systems/Execution/Engine.h"

namespace Voxel
{
	std::unique_ptr<VoxelStore> VoxelStore::_instance{};
	VoxelStore::Accessor VoxelStore::Instance{};

	const VoxelBlock VoxelStore::EmptyBlock = VoxelBlock{ };
	const BlockDescription VoxelStore::EmptyBlockDesc = BlockDescription{};

	void VoxelStore::LoadAtlas(std::string path)
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

	void VoxelStore::LoadBlockFile(std::string path)
	{
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
						DINFO("Skipping sequence item without or with invalid 'block-name' tag");
						continue;
					}

					desc.Name = name.Scalar();

					auto atlasNode = node["atlas"];
					if (!atlasNode || !atlasNode.IsScalar())
					{
						DINFO("Skipping block with no or invalid 'atlas' tag (currently all blocks are required to have an atlas tag, this may change when pre-stitched atlases are added)");
						continue;
					}

					desc.AtlasName = atlasNode.Scalar();

					auto isOpaqueNode = node["opaque"];
					desc.isOpaque = true;
					if (isOpaqueNode && isOpaqueNode.IsScalar())
						if (!StringHelper::IfBool(isOpaqueNode.Scalar(), &desc.isOpaque))
						{
							DINFO("Block '" + desc.Name + "' has invalid 'opaque' tag value of '" + isOpaqueNode.Scalar() + "'! (line " + std::to_string(isOpaqueNode.Mark().line) + " file '" + path + "')");
						}

					auto tex = node["textures"];
					if (!tex || (tex.IsScalar() && tex.Scalar() == "elsewhere"))
					{
						desc.AtlasName = "";
						std::fill(desc.FaceTextures.begin(), desc.FaceTextures.end(), std::string{});
					}
					else if (tex.IsMap())
					{
						auto& blockName = desc.Name;
						auto getFaceFunc = [&blockName](std::string name, const YAML::Node& n, std::string& texFileRef)
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

						getFaceFunc("pos-x", tex["pos-x"], desc.FaceTextures[(size_t)BlockFace::POS_X]);
						getFaceFunc("pos-y", tex["pos-y"], desc.FaceTextures[(size_t)BlockFace::POS_Y]);
						getFaceFunc("pos-z", tex["pos-z"], desc.FaceTextures[(size_t)BlockFace::POS_Z]);
						getFaceFunc("neg-x", tex["neg-x"], desc.FaceTextures[(size_t)BlockFace::NEG_X]);
						getFaceFunc("neg-y", tex["neg-y"], desc.FaceTextures[(size_t)BlockFace::NEG_Y]);
						getFaceFunc("neg-z", tex["neg-z"], desc.FaceTextures[(size_t)BlockFace::NEG_Z]);
						_unstitched.emplace_back(desc);
					}
					else
					{
						DINFO("Block '" + desc.Name + "' in block file '" + path + "' contains an invalid textures tag");
						continue;
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

	void VoxelStore::LoadAtlasDirectory(std::string directory)
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

	void VoxelStore::LoadBlockDirectory(std::string directory)
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

	void VoxelStore::StitchUnstitched(std::string faceTexDir)
	{
		// Currently this must be called once and must be called upon creation
		if (_unstitched.empty())
			return;

		UnStitchedAtlasSet unstitched{ "global", std::move(_unstitched) };

		StitchAndLoadAtlas(std::move(unstitched), faceTexDir);
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

	VoxelBlock& VoxelStore::GetOrCreateBlock(std::string blockName)
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

	VoxelStore::VoxelStore(std::string prestitchedDirectory, std::string blockDirectory, std::string faceTexDir, std::vector<UnStitchedAtlasSet> builtInAtlases)
	{
		LoadBlock(BlockDescription{ "empty", "", false }); // Load empty block description first
		LoadBlockDirectory(blockDirectory);
		//LoadAtlasDirectory(prestitchedDirectory); No! Not Yet My Boi! (not implemented)

		StitchUnstitched(faceTexDir);

		for (int i = 0; i < builtInAtlases.size(); ++i)
		{
			StitchAndLoadAtlas(builtInAtlases[i], faceTexDir);
		}
	}

	void VoxelStore::StitchAndLoadAtlas(const UnStitchedAtlasSet& set, std::string faceTexDir)
	{
		if (set.Blocks.empty())
			return;

		{
			auto existing = GetAtlas(set.AtlasGroup);
			if (existing)
			{
				DWARNING("Overwriting Existing atlas '" + set.AtlasGroup + "'!");
			}
		}

		auto ptr = std::make_shared<StitchedAtlasSet>(StichAtlas(set, faceTexDir));

		_atlasLookup[set.AtlasGroup] = ptr;

		for (auto& block : ptr->Blocks)
		{
			auto& desc = GetOrCreateBlock(block.first);
			
			for (int i = 0; i < 6; ++i)
				desc.FaceTexCoords[i] = block.second.FaceTexCoords[i];
		}
	}

	StitchedAtlasSet VoxelStore::StichAtlas(const UnStitchedAtlasSet& set, std::string faceTexDir)
	{
		if (set.Blocks.empty())
			return StitchedAtlasSet{ set.AtlasGroup, nullptr, std::unordered_map<std::string, VoxelBlock>{} };

		MidStitchData data{};
		int faceSize = DefaultFaceSize;
		float faceSizef = (float)DefaultFaceSize;

		float atlasLayerSize = (float)faceSize * DefaultAtlasLayerFaceCount + (DefaultAtlasLayerFaceCount - 1) * 2;
		int faceCount = DefaultAtlasLayerFaceCount;

		StitchedAtlasSet stitched;
		stitched.AtlasName = set.AtlasGroup;
		stitched.Image = std::make_shared<Drawing::Image2DArray>( (size_t)faceCount * faceSize + (faceCount - 1) * 2, (size_t)faceCount * faceSize + (faceCount - 1) * 2, 1 );
		stitched.Blocks = std::unordered_map<std::string, VoxelBlock>();

		auto& img = stitched.Image;

		VoxelBlock block;
		block.AtlasName = set.AtlasGroup;

		auto addFaceFunc = [&data, &faceSize, &faceSizef, &faceCount, &atlasLayerSize, &faceTexDir, &img](FaceTexCoord& face, const std::string& faceFile)
		{
			auto p = std::filesystem::path(faceFile);
			if (p.is_relative())
				p = faceTexDir / p;
			SimpleSurface faceSurf = Drawing::SDLImage::LoadSurface(p.string());
			if (!faceSurf)
				return;
			SDL_Rect dst;
			dst.x = data.curX * faceSize + data.curX * 2;
			dst.y = data.curY * faceSize + data.curY * 2;
			dst.w = faceSize;
			dst.h = faceSize;
			img->SetArea(faceSurf.Get(), dst, data.curLayer);

			if (data.curX > 0)
				img->SetArea(faceSurf.Get(), SDL_Rect{ 0, 0, 1, faceSize }, SDL_Rect{ dst.x - 1, dst.y, 1, faceSize }, data.curLayer);
			if (data.curY > 0)
				img->SetArea(faceSurf.Get(), SDL_Rect{ 0, 0, faceSize, 1 }, SDL_Rect{ dst.x, dst.y - 1, faceSize, 1 }, data.curLayer);
			if (data.curX < faceCount - 1)
				img->SetArea(faceSurf.Get(), SDL_Rect{ faceSize - 1, 0, 1, faceSize }, SDL_Rect{ dst.x + faceSize, dst.y, 1, faceSize }, data.curLayer);
			if (data.curY < faceCount - 1)
				img->SetArea(faceSurf.Get(), SDL_Rect{ 0, faceSize - 1, faceSize, 1 }, SDL_Rect{ dst.x, dst.y + faceSize, faceSize, 1 }, data.curLayer);

			face.LowerTexCoord = { (float)dst.x / atlasLayerSize, (float)dst.y / atlasLayerSize, (float)data.curLayer };
			face.UpperTexCoord = face.LowerTexCoord + floaty3{ faceSizef / atlasLayerSize, faceSizef / atlasLayerSize, 0.f };

			if (++data.curX >= faceCount)
			{
				data.curX = 0;
				if (++data.curY >= faceCount)
				{
					img->AddLayer();
					++data.curLayer;
					data.curY = 0;
				}
			}
		};
		
		for (int i = 0; i < set.Blocks.size(); ++i)
		{
			auto& desc = set.Blocks[i];

			block.Name = desc.Name;
			for (int j = 0; j < 6; ++j)
				addFaceFunc(block.FaceTexCoords[j], desc.FaceTextures[j]);

			stitched.Blocks[desc.Name] = block;
		}

		stitched.Image->LoadGL();
		stitched.Image->GenerateMipmaps();

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
		vox.isOpaque = desc.isOpaque;
		
		// Set other properties for blocks here
		// FaceTexCoords are set when loading an atlas
	}

	std::shared_ptr<StitchedAtlasSet> VoxelStore::GetAtlas(std::string name) const
	{
		auto it = _atlasLookup.find(name);
		if (it == _atlasLookup.end())
			return nullptr;
		return it->second;
	}

	bool VoxelStore::HasBlock(std::string name) const
	{
		auto it = _descriptionLookup.find(name);
		return it != _descriptionLookup.end() && it->second < _descriptions.size();
	}

	const VoxelBlock* VoxelStore::GetBlock(std::string name) const
	{
		auto lookupIt = _descriptionLookup.find(name);
		if (lookupIt == _descriptionLookup.end())
			return nullptr;

		auto id = lookupIt->second;
		if (id >= _descriptions.size())
			return nullptr;

		return &_descriptions[id];
	}

	bool VoxelStore::TryGetDescription(std::string name, VoxelBlock& desc) const
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

	VoxelBlock VoxelStore::GetDescOrEmpty(size_t ID) const
	{
		VoxelBlock out;
		if (!TryGetDescription(ID, out))
			return EmptyBlock;

		return out;
	}

	size_t VoxelStore::GetIDFor(std::string blockName) const
	{
		auto it = _descriptionLookup.find(blockName);
		if (it != _descriptionLookup.end() && it->second < _descriptions.size())
			return it->second;

		return 0;
	}

	void VoxelStore::InitializeVoxelStore(std::string prestitchedDir, std::string blockDir, std::string faceTexDir, std::vector<UnStitchedAtlasSet> builtInAtlases)
	{
		if (_instance)
			return;

		_instance = std::make_unique<VoxelStore>(prestitchedDir, blockDir, faceTexDir, builtInAtlases);
	}

	std::unique_ptr<ICube> VoxelStore::CreateCube(size_t ID) const
	{
		extern std::unique_ptr<Engine::IEngine> g_Engine;

		if (ID == 0)
			return nullptr;

		VoxelBlock desc;
		if (TryGetDescription(ID, desc))
			return std::make_unique<VoxelCube>(nullptr, &g_Engine->Resources, nullptr, floaty3{ 0.f, 0.f, 0.f }, 0, 0, 0, desc.Name);
		return nullptr;
	}
}
