#pragma once

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

		void LoadImage(std::string fileName);
		void LoadImageDirectory(std::string directory);
	public:
		TextureStore(std::string textureDirectory);

		/**
		* Returns a stored GLImage shared ptr if it exists, else an empty shared ptr.
		* Will return a Voxel Atlas ptr if the name is prefixed with 'atlas-' and what follows the prefix is an existing atlas name.
		*/
		std::shared_ptr<GLImage> GetTexture(std::string name);

		static void InitializeStore(std::string textureDirectory);
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
		std::shared_ptr<GLImage> _texture;

	public:
		TextureReference() = default;
		TextureReference(std::string textureName);
		TextureReference(std::shared_ptr<GLImage> texture); // Creates an unnamed texture reference
		TextureReference(TextureReference&& other);
		
		TextureReference& operator=(const std::string& textureName);
		TextureReference& operator=(TextureReference&& other);

		void Reset();

		inline bool HasTexture() const { return _texture.get(); }
		std::shared_ptr<GLImage> GetTexture();
		std::shared_ptr<GLImage> GetTexture() const;
	};
}