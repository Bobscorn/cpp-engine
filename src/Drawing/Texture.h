#pragma once

#include "Drawing/Image.h"

#include <string>
#include <unordered_map>

namespace Drawing
{

	// Most likely to implement lazy loading
	class TextureStore
	{
		static std::unique_ptr<TextureStore> _instance;
	public:
		class Accessor
		{
		public:
			inline operator TextureStore& () const { return *_instance; }
			inline TextureStore& operator()() const { return *_instance; }
		} static Instance;

	private:
		struct ImageState
		{
			std::string Filename;
			std::string Name;
			std::shared_ptr<SDLImage> Image;
		};

		struct LoadOpts
		{
			bool LazyLoad;
		};

		std::unordered_map<std::string, ImageState> _store;

		void LoadImage(std::string fileName);
		void LoadImageDirectory(std::string directory);
	public:
		TextureStore(std::string textureDirectory);

		std::shared_ptr<SDLImage> GetTexture(std::string name);

		static void InitializeStore(std::string textureDirectory);
	};


	class TextureReference
	{
		std::string _textureName;
		std::shared_ptr<SDLImage> _texture;

	public:
		TextureReference() = default;
		TextureReference(std::string textureName);
		TextureReference(TextureReference&& other);
		
		TextureReference& operator=(const std::string& textureName);
		TextureReference& operator=(TextureReference&& other);

		void Reset();

		inline bool HasTexture() const { return _texture.get(); }
		std::shared_ptr<SDLImage> GetTexture();
	};
}