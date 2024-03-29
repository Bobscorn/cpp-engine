/**
* Image2DArray.h
* Contains the Image2DArray class.
* 
* The Image2DArray class is used to maintain a CPU and GPU copy of an OpenGL GL_TEXTURE_2D_ARRAY
* 
*/

#pragma once

#include "Helpers/SDLHelper.h"
#include "Helpers/GLHelper.h"

#include "Drawing/Image.h"

#include <vector>

namespace Drawing
{
	// Will verify that the surface given is of the specified format
	// Will replace the surface with a copy of the specified format if it is not
	void VerifySurfaceIsFormat(SDL_Surface** surf, Uint32 format);

	class Image2DArray : public GLImage
	{
		std::vector<SDL_Surface*> _cpuSurfaces;

		GLsizei _width;
		GLsizei _height;

		SDL_Surface* CreateSurface();
		void VerifySurfaceFormats();

	public:
		Image2DArray();
		Image2DArray(GLsizei width, GLsizei height, size_t initialLayers = 0);
		Image2DArray(std::vector<SDL_Surface*>&& surfaces);
		Image2DArray(Image2DArray&& other);

		Image2DArray(const Image2DArray& other) = delete;

		virtual ~Image2DArray();

		virtual void Destroy() override;

		Image2DArray& operator=(Image2DArray&& other);
		Image2DArray& operator=(const Image2DArray& other) = delete;

		/**
		* Copy pixels from an SDL_Surface to a specified layer.
		* Does nothing if the SDL_Surface is not the same dimensions, or the specified layer is out of bounds.
		* This will also update the OpenGL representation if it has been loaded
		*/
		void SetSurface(SDL_Surface* surf, int targetLayer);

		/**
		* Copy pixels from a source SDL_Surface to a specified rect in a specified layer.
		* Does nothing if specified rect is too large, out of bounds or 0 dimensions, or the specified layer is out of bounds.
		* This will also update the OpenGL representation if it has been loaded
		*/
		void SetArea(SDL_Surface* src, SDL_Rect dstRect, int targetLayer);

		/**
		* Fills the specified rects with the target color.
		* The target color can be generated with SDL_MapRGBA.
		* The color must match the internal format of the surfaces.
		*/
		void SetRects(std::vector<SDL_Rect> rects, int targetLayer, Uint32 color);

		/**
		* Same as SetArea(SDL_Surface*, SDL_Rect, int) but also uses a srcRect to specify which part of source image to copy.
		*/
		void SetArea(SDL_Surface* src, SDL_Rect srcRect, SDL_Rect dstRect, int targetLayer);

		/**
		* Adds a layer to the CPU image. Will re-create the OpenGL image if GL is loaded
		*/
		void AddLayer();

		/**
		* Ensure that the _cpuSurfaces contains the data that the GPU currently has.
		* Use in the case that the CPU copy of pixel data has been freed and you need it back from the GPU.
		* Essentially just copies all GPU data into CPU memory.
		* 
		* Use before manipulating cpu textures if you know it only exists on the GPU
		*/ 
		void EnsureCPU();

		/**
		* Creates the GL_TEXTURE_2D_ARRAY texture and uploads the pixels.
		* Does nothing if OpenGL has already been loaded.
		*/
		void LoadGL() override;
		void UnLoadGL();

		void GenerateMipmaps();

		bool HasLoadedGL();

		inline virtual GLenum GetTarget() const override { return GL_TEXTURE_2D_ARRAY; }
	};
}