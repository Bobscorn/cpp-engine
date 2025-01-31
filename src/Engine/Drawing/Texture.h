#pragma once

#include "Drawing/AtlasTypes.h"
#include "Drawing/Image.h"
#include "Drawing/Image2DArray.h"

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
			std::shared_ptr<GLImage> Image;
		};

		struct LoadOpts
		{
			bool LazyLoad;
		};

		std::unordered_map<std::string, ImageState> _store;

		void LoadImage(const std::string& fileName);
		void LoadImageDirectory(const std::string& directory);
	public:
		TextureStore(const std::string& textureDirectory);

		/**
		* Returns whether a texture of a specific name exists and is not empty
		*/
		bool ContainsTexture(const std::string& name) const;

		/**
		* Returns a stored GLImage shared ptr if it exists, else an empty shared ptr.
		* Will return a Voxel Atlas ptr if the name is prefixed with 'atlas-' and what follows the prefix is an existing atlas name.
		*/
		bool TryGetTexture(const std::string& name, std::shared_ptr<GLImage>& out);

		/**
		* Adds a texture to the store if it does not already exist
		* Returns whether it successfully added a new texture, this depends on whether the input is not empty and whether there is an existing texture with the given name
		*/
		bool AddTexture(const std::string& name, std::shared_ptr<GLImage> in);

		static void InitializeStore(const std::string& textureDirectory);
	};


	/**
	* A class that lazily references a *named* texture.
	* The name does not refer to the filename, rather it is intended to allow switching a set of textures all referring to the same 'name'.
	* However the name of an image may coincide with the filename of the image that is loaded (ie an image named 'grass_diffuse' may be populated with the image 'grass_diffuse.png' from the disk)
	* It is lazy af the texture it stores remains null until it is retrieved via GetTexture().
	* 
	* Also Note that Voxel Atlases can be referenced via this class, they just an 'atlas-' prefix.
	*/
	class TextureReference
	{
		std::string _textureName;
		Voxel::AtlasTextureName _atlasName;
		mutable std::shared_ptr<GLImage> _texture; // A mutable cache of TextureStore::Get(_textureName)

	public:
		TextureReference() = default;
		TextureReference(std::string textureName);
		TextureReference(Voxel::AtlasTextureName atlasName); // Create a reference to an Atlas Texture
		TextureReference(std::shared_ptr<GLImage> texture); // Creates an unnamed texture reference
		TextureReference(TextureReference&& other);
		TextureReference(const TextureReference& other);
		
		TextureReference& operator=(const std::string& textureName);
		TextureReference& operator=(TextureReference&& other);
		TextureReference& operator=(const TextureReference& other);

		void Reset();

		inline bool HasTexture() const { return _texture.get(); }
		const std::shared_ptr<GLImage>& GetTexture() const;
	};
}