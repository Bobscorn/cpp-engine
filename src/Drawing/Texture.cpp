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

	void TextureStore::LoadImage(std::string fileName)
	{
		// TODO: Load a .meta file or similar to determine if a texture should be lazy-loaded
		std::filesystem::path p = fileName;
		auto name = p.filename().string();

		std::shared_ptr<SDLImage> img = std::make_shared<SDLImage>(Drawing::ImageInfo{ nullptr, name }, SDLImage::LoadSurface(fileName), 4);

		_store[name] = ImageState{ fileName, name, std::move(img) };
	}

	void TextureStore::LoadImageDirectory(std::string directory)
	{
		std::array<std::string, 4> imgExtensions = { ".png", ".jpg", ".jpeg", ".bmp" };
		for (auto& entry : std::filesystem::directory_iterator{ directory })
		{
			if (entry.exists() && entry.is_regular_file())
			{
				entry.path().extension();
				if (std::find(imgExtensions.begin(), imgExtensions.end(), entry.path().extension().string()) != imgExtensions.end())
				{
					LoadImage(entry.path().string());
				}
			}
		}
	}

	TextureStore::TextureStore(std::string textureDirectory)
	{
		LoadImageDirectory(textureDirectory);
	}

	std::shared_ptr<GLImage> TextureStore::GetTexture(std::string name)
	{
		if (name.size() > 6 && name.rfind("atlas-", 0) == 0)
		{
			auto atlas = Voxel::VoxelStore::Instance().GetAtlas(name.substr(6));
			if (atlas)
				return atlas->Image;
		}

		auto it = _store.find(name);
		if (it == _store.end())
			return nullptr;

		if (!it->second.Image)
		{
			it->second.Image = std::make_shared<SDLImage>(ImageInfo{ nullptr, it->second.Name }, SDLImage::LoadSurface(it->second.Filename), 4);
		}

		return it->second.Image;
	}

	void TextureStore::InitializeStore(std::string textureDirectory)
	{
		if (_instance)
			return;

		_instance = std::make_unique<TextureStore>(textureDirectory);
	}

	TextureReference::TextureReference(std::string textureName)
		: _textureName(textureName)
		, _texture()
	{
	}

	TextureReference::TextureReference(TextureReference&& other)
		: _textureName(std::move(other._textureName))
		, _texture(std::move(other._texture))
	{
		other._texture = nullptr;
		other._textureName.clear();
	}

	TextureReference& TextureReference::operator=(const std::string& textureName)
	{
		Reset();
		_textureName = textureName;

		return *this;
	}

	TextureReference& TextureReference::operator=(TextureReference&& other)
	{
		Reset();

		_texture = std::move(other._texture);
		_textureName = std::move(other._textureName);
		other._texture = nullptr;
		other._textureName.clear();

		return *this;
	}

	void TextureReference::Reset()
	{
		_texture = nullptr;
		_textureName.clear();
	}

	std::shared_ptr<GLImage> TextureReference::GetTexture()
	{
		if (!_texture)
			_texture = TextureStore::Instance().GetTexture(_textureName);

		return _texture;
	}

	std::shared_ptr<GLImage> TextureReference::GetTexture() const
	{
		if (_texture)
			return _texture;

		return TextureStore::Instance().GetTexture(_textureName);
	}
}
