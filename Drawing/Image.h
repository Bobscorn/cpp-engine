#pragma once

#include "Helpers/TransferHelper.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/ExceptionHelper.h"
#include "Helpers/SDLHelper.h"

#include "Systems/Execution/ResourceHolder.h"

#include <string>
#include <memory>

namespace Drawing
{
	struct ImageDeleter
	{
		void operator()(SDL_Texture *tex) const noexcept { SDL_DestroyTexture(tex); }
	};

	struct ImageInfo
	{
		CommonResources *resources;
		std::string Name;
	};

	struct SDLImage : FullResourceHolder
	{
		SDLImage(ImageInfo info, SDL_Surface *tex = nullptr, unsigned int channels = 4) : FullResourceHolder(info.resources), Name(info.Name), Tex(tex), ChannelCount(channels) {}
		virtual ~SDLImage() { Destroy(); }

		inline bool CanDraw() const noexcept { return Tex; }

		inline void Destroy()
		{
			if (Tex) 
				SDL_FreeSurface(Tex); 
			Tex = nullptr; 
			if (GLTexName)
				glDeleteTextures(1, &GLTexName);
			GLTexName = 0;
		}

		inline void Reset(SDL_Surface *tex) 
		{ 
			Destroy(); 
			Tex = tex;
		}
		inline void Reset(std::nullptr_t) { Destroy(); }
		inline SDLImage &operator=(SDL_Surface *tex) { Destroy(); Tex = tex; return *this; }
		inline SDLImage &operator=(std::nullptr_t) { Destroy(); return *this; }

		inline explicit operator bool() const noexcept { return Tex; }

		inline int GetWidth() const noexcept { return (Tex ? Tex->w : 0); }
		inline int GetHeight() const noexcept { return (Tex ? Tex->h : 0); }
		inline float GetWidthScale() const noexcept { return WidthScale; } // Returns the scale of the OpenGL texture's size to the real texture's size, always less than 1
		inline float GetHeightScale() const noexcept { return HeightScale; }
		inline GLuint GetGLTexName() const noexcept { return GLTexName; }
		inline bool HasGLTex() const noexcept { return GetGLTexName(); }

		static SDL_Texture *LoadTexture(SDL_Renderer *ren, std::string fromfile);
		static SDL_Surface *LoadSurface(std::string file);
		static GLuint LoadGLTexture(SDL_Surface *surface, float *WidthScale = nullptr, float *HeightScale = nullptr, unsigned int channels = 4);
		static GLuint LoadGLTexture(std::string file, float *WidthScale = nullptr, float *HeightScale = nullptr, unsigned int channels = 4);
		inline void LoadGL()
		{
			if (!Tex)
				return;
			if (!HasGLTex())
			{
				DINFO("Loading '" + Name + "' into OpenGL");
				GLTexName = LoadGLTexture(Tex, &WidthScale, &HeightScale, ChannelCount);
			}
		}

	protected:

		std::string Name;
		SDL_Surface *Tex{ nullptr };
		GLuint GLTexName{ 0u };
		float WidthScale{ 1.f };
		float HeightScale{ 1.f };
		unsigned int ChannelCount = 4u;
	};

	struct SDLFileImage : SDLImage
	{
		SDLFileImage(CommonResources *resources) : SDLImage({ resources, "Unknown" }) {}
		SDLFileImage(CommonResources *resources, Stringy filename, unsigned int channels = 4) : SDLImage({ resources, filename }, LoadSurface(filename), channels), Filename(filename) {}
		
		inline void LoadFromFile(Stringy filename, unsigned int channels = 4)
		{
			Reset(LoadSurface(filename));
			ChannelCount = channels;
		}

	protected:
		Stringy Filename;
	};
}
