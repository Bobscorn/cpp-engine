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

	/**
	* Base class used for textures
	*/
	class GLImage
	{
	protected:
		GLuint _tex;

	public:
		GLImage() : _tex(0) {}
		GLImage(GLuint tex) : _tex(tex) {}
		GLImage(GLImage&& other) : _tex(other._tex) { other._tex = 0; }
		GLImage(const GLImage& other) = delete;
		virtual ~GLImage() { this->Destroy(); }

		GLImage& operator=(const GLImage& other) = delete;
		inline GLImage& operator=(GLImage&& other)
		{
			Reset(other.Release());
			return *this;
		}

		inline virtual void Destroy()
		{
			if (_tex)
				glDeleteTextures(1, &_tex);
			_tex = 0;
		}

		inline void Reset(GLuint tex = 0)
		{
			Destroy();
			_tex = tex;
		}

		inline GLuint Release()
		{
			auto tmp = _tex;
			_tex = 0;
			return tmp;
		}

		inline virtual void LoadGL() {}

		inline GLuint Get() const { return _tex; }

		virtual GLenum GetTarget() const = 0;
	};

	struct SDLImage : public GLImage, FullResourceHolder
	{
		SDLImage(ImageInfo info, SDL_Surface *tex = nullptr, unsigned int channels = 4) : FullResourceHolder(info.resources), Name(info.Name), Tex(tex), ChannelCount(channels) {}
		virtual ~SDLImage() { Destroy(); }

		inline bool CanDraw() const noexcept { return Tex; }

		inline void Destroy()
		{
			if (Tex) 
				SDL_FreeSurface(Tex); 
			Tex = nullptr;
			GLImage::Destroy();
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
		inline bool HasGLTex() const noexcept { return Get(); }

		static SDL_Texture *LoadTexture(SDL_Renderer *ren, std::string fromfile);
		static SDL_Surface *LoadSurface(std::string file);
		static GLuint LoadGLTexture(SDL_Surface *surface, float *WidthScale = nullptr, float *HeightScale = nullptr, unsigned int channels = 4);
		static GLuint LoadGLTexture(std::string file, float *WidthScale = nullptr, float *HeightScale = nullptr, unsigned int channels = 4);
		inline void LoadGL() override
		{
			if (!Tex)
				return;
			if (!HasGLTex())
			{
				DINFO("Loading '" + Name + "' into OpenGL");
				_tex = LoadGLTexture(Tex, &WidthScale, &HeightScale, ChannelCount);
			}
		}

		inline virtual GLenum GetTarget() const override { return GL_TEXTURE_2D; }

	protected:

		std::string Name;
		SDL_Surface *Tex{ nullptr };
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
