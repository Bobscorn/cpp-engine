#include "Texture.h"

#include "Drawing/VoxelStore.h"

#include <filesystem>
#include <string>
#include <array>
#include <algorithm>

namespace Drawing
{
	std::unique_ptr<TextureStore> TextureStore::_instance = nullptr;
	TextureStore::Accessor TextureStore::Instance;

	void TextureStore::LoadImage(const std::string& fileName)
	{
		// TODO: Load a .meta file or similar to determine if a texture should be lazy-loaded
		// Convert to path and back to get correct filename
		std::filesystem::path p = fileName;
		auto name = p.filename().string();

		std::shared_ptr<SDLImage> img = std::make_shared<SDLImage>(Drawing::ImageInfo{ nullptr, name }, SDLImage::LoadSurface(fileName), 4);

		_store[name] = ImageState{ fileName, name, std::move(img) };
		DINFO("Loaded texture: " + fileName);
	}

	void TextureStore::LoadImageDirectory(const std::string& directory)
	{
		std::array<std::string, 4> imgExtensions = { ".png", ".jpg", ".jpeg", ".bmp" };
		if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory))
		{
			DERROR("Texture Directory does not exist!");
			DERROR("Please copy textures to the correct directory!");
			return;
		}
		for (auto& entry : std::filesystem::directory_iterator{ directory })
		{
			if (entry.exists() && entry.is_regular_file())
			{
				auto ext = entry.path().extension();
				if (std::find(imgExtensions.begin(), imgExtensions.end(), ext.string()) != imgExtensions.end())
				{
					LoadImage(entry.path().string());
				}
			}
		}
	}

	TextureStore::TextureStore(const std::string& textureDirectory)
	{
		LoadImageDirectory(textureDirectory);
	}

	bool TextureStore::TryGetTexture(const std::string& name, std::shared_ptr<GLImage>& out) 
	{
		auto it = _store.find(name);
		if (it == _store.end())
			return false;

		if (!it->second.Image)
		{
			it->second.Image = std::make_shared<SDLImage>(ImageInfo{ nullptr, it->second.Name }, SDLImage::LoadSurface(it->second.Filename), 4);
		}

		out = it->second.Image;
		return true;
	}

	void TextureStore::InitializeStore(const std::string& textureDirectory)
	{
		if (_instance)
			return;

		_instance = std::make_unique<TextureStore>(textureDirectory);
	}

	TextureReference::TextureReference(std::string textureName)
		: _textureName(textureName)
		, _atlasName()
		, _texture()
	{
	}

	TextureReference::TextureReference(Voxel::AtlasTextureName atlasName)
		: _textureName()
		, _atlasName(atlasName)
		, _texture()
	{
	}
	
	TextureReference::TextureReference(std::shared_ptr<GLImage> ptr)
		: _textureName()
		, _atlasName()
		, _texture(std::move(ptr))
	{
	}

	TextureReference::TextureReference(TextureReference&& other)
		: _textureName(std::move(other._textureName))
		, _atlasName(std::move(other._atlasName))
		, _texture(std::move(other._texture))
	{
		other._texture = nullptr;
		other._textureName.clear();
		other._atlasName = Voxel::AtlasTextureName{};
	}

	TextureReference::TextureReference(const TextureReference& other)
		: _textureName(other._textureName)
		, _atlasName(other._atlasName)
		, _texture(other._texture)
	{
	}

	TextureReference& TextureReference::operator=(const std::string& textureName)
	{
		Reset();
		_textureName = textureName;
		_atlasName = Voxel::AtlasTextureName{};

		return *this;
	}

	TextureReference& TextureReference::operator=(TextureReference&& other)
	{
		Reset();

		_texture = std::move(other._texture);
		_textureName = std::move(other._textureName);
		_atlasName = std::move(other._atlasName);
		other._texture = nullptr;
		other._textureName.clear();
		other._atlasName = Voxel::AtlasTextureName{};

		return *this;
	}

	TextureReference& TextureReference::operator=(const TextureReference& other)
	{
		Reset();

		_texture = other._texture;
		_textureName = other._textureName;
		_atlasName = other._atlasName;

		return *this;
	}

	void TextureReference::Reset()
	{
		_texture = nullptr;
		_textureName.clear();
		_atlasName = Voxel::AtlasTextureName{};
	}

	const std::shared_ptr<GLImage>& TextureReference::GetTexture() const
	{
		if (_texture)
			return _texture;

		if (_atlasName.AtlasName.size())
		{
			std::shared_ptr<Drawing::Image2DArray> tmp{};
			if (Voxel::VoxelStore::Instance().TryGetAtlasTexture(_atlasName, tmp))
				_texture = std::move(tmp);
		}
		else
		{
			TextureStore::Instance().TryGetTexture(_textureName, _texture);
		}
		return _texture;
	}
}
