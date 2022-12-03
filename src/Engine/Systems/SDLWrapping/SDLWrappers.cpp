#include "SDLWrappers.h"
#include "Systems/Input/Config1.h"

struct RendererWrapper
{
	inline static SDL_Renderer *Init(SDL_Window *win, int index, Uint32 flags) noexcept { return SDL_CreateRenderer(win, index, flags); }
	void operator()(SDL_Renderer *ren) const noexcept { SDL_DestroyRenderer(ren); }
};

struct WindowWrapper
{
	inline static SDL_Window *Init(const char *title, int x, int y, int w, int h, Uint32 flags) noexcept { return SDL_CreateWindow(title, x, y, w, h, flags); }
	inline void operator()(SDL_Window *win) const noexcept { SDL_DestroyWindow(win); }
};

/*struct FontWrapper
{
	inline static TTF_Font *Init(const char *file, int size) noexcept { return TTF_OpenFont(file, size); }
	inline void operator()(TTF_Font *fon) const noexcept { TTF_CloseFont(fon); }
};*/

void SDLW::Font::ChangeSize(float newsize)
{
	Create(newsize);
}

void SDLW::Font::ScaleSize(float scaleby)
{
	Create(desc.size * scaleby);
}

extern void test_memory();

SDLW::Window::Window(Stringy workdir, Stringy configfile, Uint32 flags) : WindowConfig(workdir, configfile) 
{ 
	if (Maximized)
		flags |= SDL_WINDOW_MAXIMIZED;
	else if (FullScreen)
		flags |= SDL_WINDOW_FULLSCREEN;

	Create(Title, X, Y, Width, Height, flags);
}

void SDLW::Window::Create(char const * title, int x, int y, int width, int height, Uint32 flags)
{
	Destroy();

	SetNewDims(width, height);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#if defined(_DEBUG) | defined(DEBUG)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	/* Turn on double buffering with a 24bit Z buffer.
	 * You may need to change this to 16 or 32 for your system */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	//SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	win = SDL_CreateWindow(title, x, y, width, height, flags);

	if (!win)
	{
		DERROR("Failed to create window, SDL Error: " + SDL_GetError());
		return;
	}

	GLC = SDL_GL_CreateContext(win);
	if (GLC == NULL)
	{
		DERROR("Failed to create OpenGL Context");
		throw Debug::GLExc("Failed to create OpenGL Context");
	}

	const unsigned char *ver = glGetString(GL_VERSION);
	if (ver == NULL)
	{
		DERROR("Failed to create OpenGL Context");
		throw Debug::GLExc("Failed to create OpenGL Context");
	}

	SDL_GL_MakeCurrent(win, GLC.operator SDL_GLContext());

	GLenum glewinit = glewInit();
	if (glewinit)
	{
		std::string er = (const char*)glewGetErrorString(glewinit);
		DERROR("Glew Initializing error: " + er);
		throw Debug::GLExc((std::string("Glew Initialization Error: ") + er).c_str());
	}

	auto adaptiveSupport = SDL_GL_SetSwapInterval(-1);
	if (adaptiveSupport == -1)
		SDL_GL_SetSwapInterval(1);

#if defined(_DEBUG) | defined(DEBUG)
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glDebugOutput, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

	glEnable(GL_MULTISAMPLE);
}
