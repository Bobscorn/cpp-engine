#include "Image.h"

#include "Helpers/MathHelper.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/StringHelper.h"
#include "Helpers/SDLHelper.h"

#include "Systems/SDLWrapping/SDLWrappers.h"

SDL_Texture *Drawing::SDLImage::LoadTexture(SDL_Renderer *ren, std::string fromfile)
{
	SDL_Surface *surface = LoadSurface(fromfile);
	if (!surface)
	{
		DERROR("Can't create an SDL Texture if loading surface fails");
		return nullptr;
	}

	SDL_Texture *out = SDL_CreateTextureFromSurface(ren, surface);
	
	if (!out)
	{
		DERROR("Failed to convert image '" + fromfile + "' to SDL Texture, SDL Error: " + SDL_GetError());
	}

	SDL_FreeSurface(surface);

	return out;
}

SDL_Surface * Drawing::SDLImage::LoadSurface(std::string file)
{
	SDL_Surface *loadedSurface = IMG_Load(file.c_str());

	if (!loadedSurface)
	{
		DERROR("Failed to load surface from: '" + file + "' SDL Error: " + IMG_GetError());
	}

	return loadedSurface;
}

GLuint Drawing::SDLImage::LoadGLTexture(SDL_Surface * surface, float *WidthScale, float *HeightScale, unsigned int channels)
{
	if (!surface)
	{
		DWARNING("Can not load null surface");
		return 0;
	}

	GLuint out;

	// Get various required format variables to convert the SDL_Surface into a OpenGL friendly pixel format
	int bits;
	Uint32 Rmask, Gmask, Bmask, Amask;
	if (!SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ABGR8888, &bits, &Rmask, &Gmask, &Bmask, &Amask))
	{
		DERROR("Failure getting Bit depth and RGBA masks of SDL Pixel Format, SDL Error: " + SDL_GetError());
		return 0u;
	}

	// Create temporary surface with raw pixels to give OpenGL
	int pow2width = Math::NextPow2((unsigned int)surface->w), pow2height = Math::NextPow2((unsigned int)surface->h);

	if (WidthScale)
		(*WidthScale) = ((float)surface->w) / ((float)pow2width);

	if (HeightScale)
		(*HeightScale) = ((float)surface->h) / ((float)pow2height);

	SimpleSurface tmp = SDL_CreateRGBSurface(0, pow2width, pow2height, bits, Rmask, Gmask, Bmask, Amask);

	if (!tmp)
	{
		DERROR("Failed to create blank SDL Surface for as medium for converting to OpenGL, SDL Error: " + SDL_GetError());
		return 0u;
	}

	SDL_SetSurfaceAlphaMod(tmp.Get(), 0);
	SDL_SetSurfaceBlendMode(tmp.Get(), SDL_BLENDMODE_NONE);

	SDL_BlitSurface(surface, nullptr, tmp.Get(), nullptr);

	// Now OpenGL stuff
		
	glGenTextures(1, &out);
	glBindTexture(GL_TEXTURE_2D, out);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	CHECK_GL_ERR("glTexParameteri");

	if (channels == 4)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pow2width, pow2height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
		DINFO("Loading OpenGL texture with 4 channels");
	}
	else if (channels == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pow2width, pow2height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
		DINFO("Loading OpenGL texture with 3 channels");
	}
	else if (channels == 1)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, pow2width, pow2height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
		DINFO("Loading OpenGL texture with 1 channel");
	}
	else
	{
		DWARNING("Texture Channel count of: " + std::to_string(channels) + " not supported");
		glDeleteTextures(1, &out);
		return 0;
	}
	CHECK_GL_ERR("glTexImage2D with " + std::to_string(pow2width) + "px by " + std::to_string(pow2height) + "px");
	
	return out;
}

GLuint Drawing::SDLImage::LoadGLTexture(std::string file, float *WidthScale, float *HeightScale, unsigned int channels)
{
	// Load SDL Surface from file
	SimpleSurface surface = LoadSurface(file);

	if (!surface)
	{
		DERROR("Can't convert a failed surface to OpenGL texture");
		return 0u;
	}

	return LoadGLTexture(surface.Get(), WidthScale, HeightScale, channels);
}
