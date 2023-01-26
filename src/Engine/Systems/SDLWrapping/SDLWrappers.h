#pragma once

#include "Helpers/ExceptionHelper.h"
#include "Helpers/GLHelper.h"

#include "Systems/Input/WindowConfig.h"

#include <GL/glew.h>

#ifdef __linux__
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_opengl.h>
#endif

#include <memory>

namespace SDLW
{
	/*template<class E, class W>
	struct Wrapper
	{
		template<class ... Args>
		Wrapper(Args ... args);

		[[nodiscard]] operator E*() { return _ptr.get(); }
		[[nodiscard]] E *operator->() { ptr.get(); }
		[[nodiscard]] E &operator*() { return *ptr; }

		[[nodiscard]] operator bool() const noexcept { return ptr.get(); }

		[[nodiscard]] bool operator==(E *ptr) const noexcept { return _ptr.get() == ptr; }
	protected:
		std::unique_ptr<E, W> _ptr;
	};*/

	struct FontDesc
	{
		std::string file;
		float size;

		inline bool operator==(FontDesc const &other) const noexcept { return (file == other.file) && (size == other.size); }
	};

	struct Font
	{
		Font() : desc({ "", 0 }), font(nullptr) {}
		explicit Font(FontDesc desc) : desc(desc) { Create(desc.size); }
		Font(Font&& f) noexcept : desc(f.desc), font(f.font) { f.font = nullptr; }
		Font(const Font &other) : desc(other.desc), font(nullptr) { if (other.font) Create(desc.size); }
		~Font() { Destroy(); }

		inline Font &operator=(const Font &other)
		{
			this->desc = other.desc;
			if (other.font)
				Create(desc.size);
			return *this;
		}
		Font & operator=(Font&& f) noexcept
		{
			Destroy();
			desc = f.desc;
			font = f.font;
			f.font = nullptr;
			return *this;
		}

		void ChangeSize(float newsize);
		void ScaleSize(float scaleby);

		[[nodiscard]] inline FontDesc GetDesc() const noexcept { return desc; }
		[[nodiscard]] inline TTF_Font *GetFont() const noexcept { return font; }

		inline explicit operator bool() const noexcept { return font; }

		inline void Create(float ptsize)
		{
			Destroy();
			desc.size = fmaxf(ptsize, 0.f);
			std::string finalfile = (std::string(FontPrefix) + desc.file);
			DINFO("Loading font from file: '" + finalfile + "'");
			font = TTF_OpenFont(finalfile.c_str(), int(std::roundf(desc.size)));
			if (!font) 
			{ 
				DERROR("Failed to create font from file: '" + finalfile + "' with size '" + std::to_string(desc.size) + "' SDL Error: '" + TTF_GetError()); 
			}
		}
		inline void Destroy()
		{
			if (font)
				TTF_CloseFont(font);
			font = nullptr;
		}

	protected:
		static constexpr const char *FontPrefix = "Fonts/";

		FontDesc desc;
		TTF_Font * font{ nullptr };
	};

	struct GLContext
	{
		GLContext() : GLC(NULL) {}
		GLContext(SDL_Window *win) : GLC(Create(win)) {}
		GLContext(SDL_GLContext e) : GLC(e) {}
		GLContext(GLContext &&other) noexcept : GLC(other.GLC) { other.GLC = NULL; }
		~GLContext() { Reset(); }

		inline void Reset(SDL_GLContext e = NULL)
		{
			SDL_GL_DeleteContext(GLC);
			GLC = e;
		}

		inline GLContext &operator=(SDL_GLContext e)
		{
			Reset(e);
			return *this;
		}

		inline GLContext &operator=(GLContext &&other) noexcept
		{
			Reset(other.GLC);
			other.GLC = NULL;
			return *this;
		}

		inline SDL_GLContext Get() const noexcept { return GLC; }

		inline bool operator==(SDL_GLContext e) const noexcept { return GLC == e; }
		//inline bool operator==(void *num) const noexcept { return GLC == (SDL_GLContext)num; }

		inline explicit operator SDL_GLContext() const noexcept { return GLC; }
		inline explicit operator bool() const noexcept { return GLC; }

		inline static SDL_GLContext Create(SDL_Window *win)
		{
			SDL_GLContext e = SDL_GL_CreateContext(win);
			if (!e)
				throw Debug::SDLException("Failed to create OpenGL Context from Window");
			return e;
		}

	protected:
		SDL_GLContext GLC = NULL;
	};

	struct Window : Config1::WindowConfig
	{
		Window(Stringy workdir, Stringy configfile, Uint32 flags);
		Window(char const * title, int x, int y, int width, int height, Uint32 flags) { Create(title, x, y, width, height, flags); }
		~Window() { Destroy(); }

		[[nodiscard]] SDL_Window *Get() const noexcept { return win; }

		void Flip() const
		{
			SDL_GL_SwapWindow(win);
		}

		void Create(char const * title, int x, int y, int width, int height, Uint32 flags);
		inline void Destroy()
		{
			if (win)
				SDL_DestroyWindow(win);
			win = nullptr;
			GLC.Reset();
		}

		void SetNewDims(int width, int height)
		{
			Width = width;
			Height = height;
			InverseWindowWidth = (Width <= 0 ? 0.f : 1.f / Width);
			InverseWindowHeight = (Height <= 0 ? 0.f : 1.f / Height);
			CenterX = int(std::lroundf(0.5f * width));
			CenterY = int(std::lroundf(0.5f * height));
			AspectRatio = float(width) / float(height);
		}

		[[nodiscard]] inline int GetWidth() const noexcept { return Width; }
		[[nodiscard]] inline int GetHeight() const noexcept { return Height; }
		[[nodiscard]] inline const int *GetPCenterX() const noexcept { return &CenterX; }
		[[nodiscard]] inline const int *GetPCenterY() const noexcept { return &CenterY; }
		[[nodiscard]] inline const float *GetPAspectRatio() const noexcept { return &AspectRatio; }
		[[nodiscard]] inline const int *GetPWinWidth() const noexcept { return &Width; }
		[[nodiscard]] inline const int *GetPWinHeight() const noexcept { return &Height; }
		[[nodiscard]] inline const float *GetPInvWidth() const noexcept { return &InverseWindowWidth; }
		[[nodiscard]] inline const float *GetPInvHeight() const noexcept { return &InverseWindowHeight; }
		[[nodiscard]] inline int *GetPCenterX() noexcept { return &CenterX; }
		[[nodiscard]] inline int *GetPCenterY() noexcept { return &CenterY; }
		[[nodiscard]] inline float *GetPAspectRatio() noexcept { return &AspectRatio; }
		[[nodiscard]] inline int *GetPWinWidth() noexcept { return &Width; }
		[[nodiscard]] inline int *GetPWinHeight() noexcept { return &Height; }
		[[nodiscard]] inline float *GetPInvWidth() noexcept { return &InverseWindowWidth; }
		[[nodiscard]] inline float *GetPInvHeight() noexcept { return &InverseWindowHeight; }
		
	protected:

		SDL_Window * win{ nullptr };
		SDLW::GLContext GLC;
		int CenterX{ 0u };
		int CenterY{ 0u };
		float AspectRatio{ 16.f / 9.f };
		float InverseWindowWidth{ 0.f };
		float InverseWindowHeight{ 0.f };
	};
}