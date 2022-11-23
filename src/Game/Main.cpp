
#include <Helpers/StringHelper.h>
#include <Helpers/VectorHelper.h>
#include <Helpers/MathHelper.h>
#include <Helpers/FileHelper.h>
#include <Helpers/ProfileHelper.h>

#include <Game/GameEngine.h>
#include <Systems/Events/Events.h>
#include <Systems/Input/Input.h>
#include <Systems/Input/Assimp.h>
#include <Systems/Audio/Audio.h>

#include <Systems/SDLWrapping/SDLWrappers.h>

#include <Config/Config.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <AL/al.h>
#include <AL/alc.h>

#ifdef WIN32
#	ifndef _WINDOWS_
#		define WIN32_MEAN_AND_LEAN
#		include <Windows.h>
#	endif
#endif

#include <memory>
#include <iostream>
#include <exception>
#include <random>
#include <sstream>

#include <Game/Scene/StartingScene.h>
#include "ParkourScene.h"

#define _CHEATS_

extern void OutputSizeTest();

std::unique_ptr<Engine::IEngine> g_Engine;

struct EventReporter
{
	struct InfoReporter : Events::IGroupListenerT<Events::InfoGroup>
	{
		inline bool Receive(Events::IEvent *event) override
		{
			DINFO("Info Event: " + Events::EventName(event->Type));
			return true;
		}
	};

	struct InputReporter : Events::IGroupListenerT<Events::InputGroup>
	{
		inline bool Receive(Events::IEvent *event) override
		{
			DINFO("Input Event: " + Events::EventName(event->Type));
			return true;
		}
	};

	struct WindowReporter : Events::IGroupListenerT<Events::WindowGroup>
	{
		inline bool Receive(Events::IEvent *event) override
		{
			DINFO("Window Event: " + Events::EventName(event->Type));
			return true;
		}
	};

	struct PhysicsReporter : Events::IGroupListenerT<Events::PhysicsGroup>
	{
		inline bool Receive(Events::IEvent *event) override
		{
			DINFO("Physics Event: " + Events::EventName(event->Type));
			return true;
		}
	};

	inline void Add(Events::EventManager *man)
	{
		man->Add({ &inf, &inp, &win, &phy });
	}

protected:
	InfoReporter inf;
	InputReporter inp;
	WindowReporter win;
	PhysicsReporter phy;
};

bool Quit = false; // Quit flag

void QuitDatAss()
{
	Quit = true;
}

void EventProc(const SDL_Event *e);

#ifdef EC_PROFILE
#	ifdef _DEBUG
#		define RUN_PROFILE_PUSH(x) g_Engine->Resources.Profile->Push(x, __FUNCTION_NAME__, __LINE__)
#		define RUN_PROFILE_POP() g_Engine->Resources.Profile->Pop(__FUNCTION_NAME__, __LINE__)
#	else
#		define RUN_PROFILE_PUSH(x) g_Engine->Resources.Profile->Push(x)
#		define RUN_PROFILE_POP() g_Engine->Resources.Profile->Pop()
#	endif
#else
#	define RUN_PROFILE_PUSH(x) ((int)0)
#	define RUN_PROFILE_POP() ((int)0)
#endif

void Run()
{
	SDL_Event e = { 0 };
	while (!Quit)
	{
#ifdef EC_PROFILE
		g_Engine->Resources.Profile->BeginFrame();
#endif 
		RUN_PROFILE_PUSH("Event Proc");
		while (SDL_PollEvent(&e))
		{
			EventProc(&e);
		}
		if (g_Engine)
			RUN_PROFILE_POP();
		if (!Quit)
		{
			RUN_PROFILE_PUSH("Update");

			g_Engine->Run();

			RUN_PROFILE_POP();
#ifdef EC_PROFILE
			(void)g_Engine->Resources.Profile->EndFrame();
#endif
		}
	}
}

int main(int argc, char *args[])
{
	(void)argc;
	(void)args;

	std::cout << "Encapsulated!" << std::endl;
	std::cout << ".." << std::endl;

	bool Init = true;

	Uint32 flags = SDL_INIT_VIDEO;
#ifndef WIN32
	flags |= SDL_INIT_TIMER;
#endif
	Init = SDL_Init(flags) >= 0;
	if (!Init)
	{
		printf("SDL Could not initialize! SDL_Error: %s\n", SDL_GetError());
	}

	int ImgFlags{ 0 };
#ifdef EC_LOAD_PNG
#pragma message ("Building with PNG Support")
	ImgFlags |= IMG_INIT_PNG;
#endif
#ifdef EC_LOAD_JPEG
#pragma message ("Building with JPG Support")
	ImgFlags |= IMG_INIT_JPG;
#endif
#ifdef EC_LOAD_TIFF
	ImgFlags |= IMG_INIT_TIF;
#endif
#ifdef EC_LOAD_WEBP
	ImgFlags |= IMG_INIT_WEBP;
#endif
	int LoadedFlags = IMG_Init(ImgFlags);
	Init = LoadedFlags == ImgFlags;
	
	if (!Init)
	{
		printf("SDL Imaging Could not Initialize. IMG Error: %s\n", IMG_GetError());
		if (LoadedFlags)
		{
			ListString ass;
			if (LoadedFlags & IMG_INIT_PNG)
				ass += "PNG Support";
			if (LoadedFlags & IMG_INIT_JPG)
				ass += "JPEG Support";
			if (LoadedFlags & IMG_INIT_TIF)
				ass += "TIFF Support";
			if (LoadedFlags & IMG_INIT_WEBP)
				ass += "WEBP Support";
			DINFO("SDL Imaging did manage to load " + ass.Finalize());
		}
	}
	else
	{
		ListString ass;
		if (LoadedFlags & IMG_INIT_PNG)
			ass += "PNG Support";
		if (LoadedFlags & IMG_INIT_JPG)
			ass += "JPEG Support";
		if (LoadedFlags & IMG_INIT_TIF)
			ass += "TIFF Support";
		if (LoadedFlags & IMG_INIT_WEBP)
			ass += "WEBP Support";
		DINFO("SDL Imaging successfully initialized with: " + ass.Finalize());
	}

	if (TTF_Init() < 0)
	{
		printf("TTF Could not Initialize with: %s\n", TTF_GetError());
	}

#if true
	
	try {
		/*
		ALCdevice *device;
		device = alcOpenDevice(NULL);

		if (!device)
		{
			DERROR("Could not obtain a device");
			throw std::exception{ "Could not obtain a device" };
		}

		ALCcontext *context;

		context = alcCreateContext(device, NULL);
		if (!alcMakeContextCurrent(context))
		{
			DERROR("Could not make context current");
			throw std::exception{ "Failed to make OpenAL Context Current" };
		}

		if (Audio::TestForALError(Audio::ALErrorGuy{ "Creating Context" }))
			throw std::exception{ "Failed to Create Context or something" };
		*/
		
		g_Engine = std::make_unique<Engine::GameEngine>();
		g_Engine->SwitchScene(std::make_unique<Parkour::ParkourStartingScene>(&g_Engine->Resources, std::unique_ptr<Parkour::IParkourDifficultyScene>(new Parkour::ParkourScene(&g_Engine->Resources, 0))));
		//g_Engine->SwitchScene(std::make_unique<Parkour::ParkourStartingScene>(&g_Engine->Resources, std::make_unique<Parkour::ParkourScene>(&g_Engine->Resources, 0)));
		DINFO("Yo whats popping my bois");
		Run();
	}
	catch (Debug::GLExc &e)
	{
		Debug::ReportError("OpenGL ran into an error", (std::string("OpenGL ran into an error: ") + e.what()).c_str());
	}
	catch (Debug::SDLException &e)
	{
		Debug::ReportError("SDL ran into an error", (std::string("SDL ran into an error: ") + e.what()).c_str());
	}

	// Destroy engine before Shutting down SDL/TTG/IMG
	g_Engine = nullptr;


	// Basic OpenGL Test

#else
#	if false

	SDL_Window *win = 0;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	
	/* Turn on double buffering with a 24bit Z buffer.
	 * You may need to change this to 16 or 32 for your system */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		
	win = SDL_CreateWindow("GL Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (win)
	{

		SDL_GLContext glc = SDL_GL_CreateContext(win);
		if (glc == NULL)
		{
			DERROR("Failed to create OpenGL Context");
			//throw Debug::GLExc("Failed to create OpenGL Context");
		}

		const unsigned char *ver = glGetString(GL_VERSION);
		if (ver == NULL)
		{
			DERROR("Failed to create OpenGL Context");
			//throw Debug::GLExc("Failed to create OpenGL Context");
		}

		SDL_GL_MakeCurrent(win, glc);

		GLenum glewinit = glewInit();
		if (glewinit)
		{
			std::string er = (const char*)glewGetErrorString(glewinit);
			DERROR("Glew Initializing error: " + er);
			//throw Debug::GLExc((std::string("Glew Initialization Error: ") + er).c_str());
		}

#if defined(_DEBUG) | defined(DEBUG)
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif

		/* Clear our buffer with a red background */
		glClearColor(1.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		/* Swap our back buffer to the front */
		SDL_GL_SwapWindow(win);
		/* Wait 2 seconds */
		SDL_Delay(2000);

		/* Same as above, but green */
		glClearColor(0.0, 1.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(win);
		SDL_Delay(2000);

		/* Same as above, but blue */
		glClearColor(0.0, 0.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(win);
		SDL_Delay(2000);

		float points[] = {
			0.f,  200.f,  0.0f,
			200.f, -200.f,  0.0f,
			-200.f, -200.f,  0.0f
		};

		float texcoords[] = {
			0.5f, 1.0f,
			1.0f, 0.0f,
			0.0f, 0.0f,
		};

		// Vertex Buffer Objects
		// Vertices
		GLuint vbo = 0;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

		// Tex Coords
		GLuint texcoord_vbo = 0;
		glGenBuffers(1, &texcoord_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), texcoords, GL_STATIC_DRAW);

		// Vertex Array Object
		GLuint vao = 0;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		// Indices/Index Buffer Object
		unsigned int indices[] = 
		{
			0, 1, 2
		};

		GLuint ibo = 0;
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3, indices, GL_STATIC_DRAW);

		const char* vertex_shader =
			"#version 400\n"
			"layout(location = 0) in vec3 vertex_position;"
			"layout(location = 1) in vec2 vertex_texcoord;"
			"uniform mat4 Projection;"
			"uniform mat4 ModelView;"
			"out vec2 texcoord;"
			"void main() {"
			"	texcoord = vertex_texcoord;"
			"	gl_Position = Projection * ModelView * vec4(vertex_position, 1.0);"
			"}";

		const char* fragment_shader =
			"#version 400\n"
			"uniform sampler2D texture_guy;"
			"in vec2 texcoord;"
			"out vec4 frag_colour;"
			"void main() {"
			"  frag_colour = texture(texture_guy, texcoord);"
			"}";

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &vertex_shader, NULL);
		glCompileShader(vs);
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &fragment_shader, NULL);
		glCompileShader(fs);

		GLuint shader_programme = glCreateProgram();
		glAttachShader(shader_programme, fs);
		glAttachShader(shader_programme, vs);

		//glBindAttribLocation(shader_programme, 0, "vertex_position");
		//glBindAttribLocation(shader_programme, 1, "vertex_colour");

		glLinkProgram(shader_programme);

		// --------------------
		// Matrix

		glUseProgram(shader_programme);

		GLuint projmat = 0;
		projmat = glGetUniformLocation(shader_programme, "Projection");
		Matrixy4x4 mat = Matrixy4x4::OrthoProject(800, 600, 0.1f, -10.f);
		glUniformMatrix4fv(projmat, 1, GL_FALSE, mat.ma);

		GLuint modelmat = 0;
		modelmat = glGetUniformLocation(shader_programme, "ModelView");
		mat = Matrixy4x4::Identity();
		glUniformMatrix4fv(modelmat, 1, GL_FALSE, mat.ma);

		// --------------------
		// Image

		// Get various required format variables to convert the SDL_Surface into a OpenGL friendly pixel format
		int bits;
		Uint32 Rmask, Gmask, Bmask, Amask;
		if (!SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ARGB8888, &bits, &Rmask, &Gmask, &Bmask, &Amask))
		{
			DERROR("Failure getting Bit depth and RGBA masks of SDL Pixel Format, SDL Error: " + SDL_GetError());
			return 0u;
		}

		SDL_Surface *img = IMG_Load("background.jpeg");
		// Create temporary surface with raw pixels to give OpenGL
		int pow2width = Math::NextPow2((unsigned int)img->w), pow2height = Math::NextPow2((unsigned int)img->h);
		SDL_Surface *tmp = SDL_CreateRGBSurface(0, pow2width, pow2height, bits, Rmask, Gmask, Bmask, Amask);

		if (!tmp)
		{
			DERROR("Failed to create blank SDL Surface for as medium for converting to OpenGL, SDL Error: " + SDL_GetError());
			return 0u;
		}

		SDL_SetSurfaceAlphaMod(tmp, 0xFF);
		SDL_SetSurfaceBlendMode(tmp, SDL_BLENDMODE_NONE);

		SDL_BlitSurface(img, nullptr, tmp, nullptr);
				
		GLuint image = 0;
		glGenTextures(1, &image);

		glBindTexture(GL_TEXTURE_2D, image);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pow2width, pow2height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		SDL_FreeSurface(tmp);
		SDL_FreeSurface(img);

		// -----------------



		
		// Looped De doo
		int ass = 0;
		while (++ass < 500)
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(shader_programme);
			glBindVertexArray(vao);

			Matrixy4x4 crass = Matrixy4x4::Translate((float)ass * 0.5f, (float)ass * 0.25f, 0);
			glUniformMatrix4fv(modelmat, 1, GL_FALSE, crass.ma);

			// draw points 0-3 from the currently bound VAO with current in-use shader
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
			// put the stuff we've been drawing onto the display
			SDL_GL_SwapWindow(win);
		}

		/* Delete our opengl context, destroy our window, and shutdown SDL */
		SDL_GL_DeleteContext(glc);

	}
	else
	{
		DERROR("Could not create window: " + SDL_GetError());
		bool wtf = true;
	}


	SDL_DestroyWindow(win);	
#	else
	
#		if false
	Matrixy4x4 ass = Matrixy3x3::RotationY(Math::NinetyDegreesRF);
	floaty3 negz = { 0.f, 0.f, -1.f };
	floaty3 posx = ass.Transform(negz);

	Matrixy4x4 nass;
	if (!Matrixy4x4::Inverted(ass, nass))
		bool ass = "Poo";

	floaty3 negx = nass.Transform(negz);

	Matrixy4x4 crass = Matrixy4x4::Translate({ 2.f, 3.f, 4.f });
	Matrixy4x4 ncrass;
	if (!Matrixy4x4::Inverted(crass, ncrass))
		bool maaannnn = "Gotem";

	bool quickmaths = true;
#		else
#			if false
	Assimp::Importer imp;

	const aiScene * ascene = imp.ReadFile("subway.dae", aiProcess_ValidateDataStructure |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices);

	PulledScene scene = PullScene(ascene);

	bool quickmaths = true;
#			else

	try
	{
		ALCdevice *device;
		device = alcOpenDevice(NULL);

		if (!device)
		{
			DERROR("Could not obtain a device");
			throw std::exception{ "Could not obtain a device" };
		}

		ALCcontext *context;

		context = alcCreateContext(device, NULL);
		if (!alcMakeContextCurrent(context))
		{
			DERROR("Could not make context current");
			throw std::exception{ "Failed to make OpenAL Context Current" };
		}

		if (Audio::TestForALError(Audio::ALErrorGuy{ "Creating Context" }))
			throw std::exception{ "Failed to Create Context or something" };

		ALfloat listenOrientation[] = { 0.f, 0.f, -1.f,  0.f, 1.f, 0.f };
		{
			Audio::ALErrorGuy poserror{ "Setting Position" };
			alListener3f(AL_POSITION, 0.f, 0.f, 0.f);
		}
		{
			Audio::ALErrorGuy velerror{ "Setting Velocity" };
			alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f);
		}
		{
			Audio::ALErrorGuy orienterror{ "Setting Orientation" };
			alListenerfv(AL_ORIENTATION, listenOrientation);
		}

		{
			Audio::Clear();
			Audio::ALSource source = Audio::CreateSauce();
			Audio::ALBufferI buf = Audio::CreateBufferFile("audio.wav");

			alSourcef(source.Get(), AL_PITCH, 1.f);
			Audio::ThrowReport();
			alSourcef(source.Get(), AL_GAIN, 1.f);
			Audio::ThrowReport();
			alSource3f(source.Get(), AL_POSITION, 0.f, 0.f, -10.f);
			Audio::ThrowReport();
			alSource3f(source.Get(), AL_VELOCITY, 0.f, 0.f, -3.f);
			Audio::ThrowReport();
			alSourcei(source.Get(), AL_LOOPING, AL_FALSE);
			Audio::ThrowReport();

			buf.AttachTo(source);

			alSourcePlay(source.Get());

			ALint state = 0;

			do
			{
				alGetSourcei(source.Get(), AL_SOURCE_STATE, &state);
			} while (state == AL_PLAYING);

			bool gotem = 2;
		}

		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}
	catch (const std::exception &e)
	{
		DERROR("Exception thrown: " + e.what());
	}
	


#			endif
#		endif
#	endif
#endif

	//Quit SDL subsystems 
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}

/*#pragma warning(suppress:4100)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#pragma warning(disable:4996)

#ifdef USE_CONSOLE
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	HWND hwnd = ::GetConsoleWindow();
	if (hwnd != NULL)
	{
		HMENU hMenu = ::GetSystemMenu(hwnd, FALSE);
		if (hMenu != NULL) DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
	}

	BOOL result = SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
	assert(result);
#endif

	//PrimaryFunc<PRIMARY_FUNC_TYPE>(nCmdShow);
#ifdef USE_CONSOLE
	FreeConsole();
#endif
#pragma warning(default:4996)
}*/



// Global end process method
void Quitty()
{
	if (g_Engine)
		//g_Engine->Delete();
	g_Engine = nullptr;

	IMG_Quit();
	SDL_Quit();
}

bool Paused{ false }, Resizing{ false }, Maximized{ false }, Minimized{ false };
bool StopWindowFocus = false;

#ifdef EC_PROFILE
#define PROFILE_PUSH_TMP(x) PROFILE_PUSH(x)
#define PROFILE_POP_TMP() PROFILE_POP()
#undef PROFILE_PUSH
#undef PROFILE_POP
#ifdef _DEBUG
#define PROFILE_PUSH(x) g_Engine->Resources.Profile->Push(x, __FUNCTION_NAME__, __LINE__)
#define PROFILE_POP() g_Engine->Resources.Profile->Pop(__FUNCTION_NAME__, __LINE__)
#else
#define PROFILE_PUSH(x) g_Engine->Resources.Profile->Push(x)
#define PROFILE_POP() g_Engine->Resources.Profile->Pop()
#endif
#endif
void EventProc(const SDL_Event *e)
{
	switch (e->type)
	{
	case SDL_WINDOWEVENT:
		PROFILE_PUSH("Window Event");
		if (!StopWindowFocus)
		{
			switch (e->window.event)
			{
			case SDL_WINDOWEVENT_HIDDEN:
				// Pause when window is inactive
				Paused = true;
			case SDL_WINDOWEVENT_LEAVE:
			{
				Event::WindowFocusEvent ass(false, *g_Engine->Resources.UpdateID);
#ifdef EC_PROFILE
				g_Engine->Send(&ass, g_Engine->Resources.Profile);
#else
				g_Engine->Send(&ass);
#endif // EC_PROFILE
				break;
			}
			case SDL_WINDOWEVENT_ENTER:
			{
				Event::WindowFocusEvent ass(true, *g_Engine->Resources.UpdateID);
#ifdef EC_PROFILE
				g_Engine->Send(&ass, g_Engine->Resources.Profile);
#else
				g_Engine->Send(&ass);
#endif // EC_PROFILE
				break;
			}
			case SDL_WINDOWEVENT_SHOWN:
			{
				// Unpause if its not inactive (assuming that means its active)
				Paused = false;
				Event::WindowFocusEvent ass(true, *g_Engine->Resources.UpdateID);
#ifdef EC_PROFILE
				g_Engine->Send(&ass, g_Engine->Resources.Profile);
#else
				g_Engine->Send(&ass);
#endif // EC_PROFILE
				break;
			}
			case SDL_WINDOWEVENT_MINIMIZED:
				Paused = true;
				// timer.stop();
				Minimized = true;
				Maximized = false;
				// Write window config to file
				g_Engine->OnMinimize();
				break;
			case SDL_WINDOWEVENT_MAXIMIZED:
				Paused = false;
				// timer.start()
				Minimized = false;
				Maximized = true;
				g_Engine->OnMaximize();
				break;
			case SDL_WINDOWEVENT_RESIZED: // Assuming RESIZED is finished fiddling with window
				Paused = false;
				Resizing = false;
				DINFO("Would just like to say a SDL_WINDOWEVENT_RESIZED was received");
				break;
			case SDL_WINDOWEVENT_SIZE_CHANGED: // Assuming SIZE_CHANGED is something in the middle of changing this windows' size
				Paused = true;
				Resizing = false;
				g_Engine->OnResize(int(e->window.data1), int(e->window.data2));
				break;
			case SDL_WINDOWEVENT_RESTORED:
				if (Minimized) { // Window was minimized

				}
				else if (Maximized) { // Window was Maximized, most likely means window was de-maximized
					Maximized = false;
					g_Engine->OnDeMaximize();
				}
				else if (Resizing) {
					// This means the user is currently resizing the window,
					// Only when the user starts resizing and stops resizing should action be taken,
					// As such nothing is done here, but instead stuff
					// is done in WM_ENTERSIZEMOVE and WM_EXITSIZEMOVE
				}
				else {

				}
				break;
			}
		}
		PROFILE_POP();
		break;

	/*case WM_DPICHANGED:
	{
		g_Engine->OnDpiChanged();
		RECT *const NewWindow = reinterpret_cast<RECT*const>(lParam);
		SetWindowPos(g_Engine->Hwnd, NULL, NewWindow->left, NewWindow->top, NewWindow->right - NewWindow->left, NewWindow->bottom - NewWindow->top, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
	}*/
		
		// WM_ENTERSIZEMOVE is sent when the user grabs the resize bars.
	case SDL_KEYDOWN:
	{
		PROFILE_PUSH("Keydown Event");
		if (e->key.repeat)
		{
			PROFILE_POP();
			return;
		}
		Event::KeyInput key = Event::KeyInput(e->key.keysym.sym, true, 0ul);// *g_Engine->Resources.FrameID);
#ifdef EC_PROFILE
		g_Engine->Send(&key, g_Engine->Resources.Profile);
#else
		g_Engine->Send(&key);
#endif // EC_PROFILE
		PROFILE_POP();
		break;
	}
	case SDL_KEYUP:
	{
		PROFILE_PUSH("Keyup Event");
#ifdef _CHEATS_
		if (e->key.keysym.sym == SDLK_F9)
		{
			StopWindowFocus = !StopWindowFocus;
			DINFO("Window Focus Events have been toggled " + (StopWindowFocus ? "Off" : "On"));
		}
		else if (e->key.keysym.sym == SDLK_F7)
		{
			Requests::Request ass("UI", { "ToggleDebugDrawing" });
			g_Engine->Req.Request(ass);
			DINFO("DebugDraw has been toggled");
		}
		else if (e->key.keysym.sym == SDLK_F8)
		{
			Requests::Request ass("UI", { "ToggleMouseMovement" });
			g_Engine->Req.Request(ass);
			ass = Requests::Request("UI", { "ToggleHover" });
			g_Engine->Req.Request(ass);
			DINFO("UI Mouse Movement and Hover has been toggled");
		}
		else if (e->key.keysym.sym == SDLK_i)
		{
			Requests::Request ass("EndLevel");
			g_Engine->Req.Request(ass);
			DINFO("Ended Level");
		}
		else if (e->key.keysym.sym == SDLK_l)
		{
			Requests::Request ass("StartSimpleProfile");
			g_Engine->Req.Request(ass);
			DINFO("Started Profile");
		}
#endif
		Event::KeyInput key = Event::KeyInput(e->key.keysym.sym, false, 0ul);// *g_Engine->Resources.FrameID);
#ifdef EC_PROFILE
		g_Engine->Send(&key, g_Engine->Resources.Profile);
#else
		g_Engine->Send(&key);
#endif // EC_PROFILE
		PROFILE_POP();
		break;
	}
		// Finally, to support our mouse input virtual functions, we will handle the following mouse messages as follows:
	case SDL_MOUSEBUTTONDOWN:
		PROFILE_PUSH("MouseButtonDown Event");
		switch (e->button.button)
		{
		case SDL_BUTTON_LEFT:
		{
			Event::MouseButton ass = Event::MouseButton(e->button.x, e->button.y, LMB, true, *g_Engine->Resources.UpdateID);
#ifdef EC_PROFILE
			g_Engine->Send(&ass, g_Engine->Resources.Profile);
#else
			g_Engine->Send(&ass);
#endif // EC_PROFILE
			break;
		}
		case SDL_BUTTON_MIDDLE:
		{
			Event::MouseWheelButton ass = Event::MouseWheelButton(e->button.x, e->button.y, true, *g_Engine->Resources.UpdateID);
#ifdef EC_PROFILE
			g_Engine->Send(&ass, g_Engine->Resources.Profile);
#else
			g_Engine->Send(&ass);
#endif // EC_PROFILE
			break;
		}
		case SDL_BUTTON_RIGHT:
		{
			Event::MouseButton ass = Event::MouseButton(e->button.x, e->button.y, RMB, true, *g_Engine->Resources.UpdateID);
#ifdef EC_PROFILE
			g_Engine->Send(&ass, g_Engine->Resources.Profile);
#else
			g_Engine->Send(&ass);
#endif // EC_PROFILE
			break;
		}
		default:
			DINFO("Unknown Mousebutton pressed (Most likely an Extended Mouse Button, not supported currently)");
			break;
		}
		PROFILE_POP();
		break;
	case SDL_MOUSEBUTTONUP:
		PROFILE_PUSH("MouseButtonUp Event");
		switch (e->button.button)
		{
		case SDL_BUTTON_LEFT:
		{
			Event::MouseButton ass = Event::MouseButton(e->button.x, e->button.y, LMB, false, 0ul);// *g_Engine->Resources.FrameID);
#ifdef EC_PROFILE
			g_Engine->Send(&ass, g_Engine->Resources.Profile);
#else
			g_Engine->Send(&ass);
#endif // EC_PROFILE
			break;
		}
		case SDL_BUTTON_MIDDLE:
		{
			Event::MouseWheelButton ass = Event::MouseWheelButton(e->button.x, e->button.y, false, 0ul);// *g_Engine->Resources.FrameID);
#ifdef EC_PROFILE
			g_Engine->Send(&ass, g_Engine->Resources.Profile);
#else
			g_Engine->Send(&ass);
#endif // EC_PROFILE
			break;
		}
		case SDL_BUTTON_RIGHT:
		{
			Event::MouseButton ass = Event::MouseButton(e->button.x, e->button.y, RMB, false, 0ul);// *g_Engine->Resources.FrameID);
#ifdef EC_PROFILE
			g_Engine->Send(&ass, g_Engine->Resources.Profile);
#else
			g_Engine->Send(&ass);
#endif // EC_PROFILE
			break;
		}
		}
		PROFILE_POP();
		break;
	case SDL_MOUSEMOTION:
	{
		PROFILE_PUSH("MouseMotion Event");
		Event::MouseMove ass = Event::MouseMove(e->motion.x, e->motion.y, 0ul);// *g_Engine->Resources.FrameID);
#ifdef EC_PROFILE
		g_Engine->Send(&ass, g_Engine->Resources.Profile);
#else	
		g_Engine->Send(&ass);
#endif
		PROFILE_POP();
		break;
	}
	case SDL_MOUSEWHEEL:
	{
		int mousex, mousey;
		SDL_GetMouseState(&mousex, &mousey);
		Event::MouseWheel ass = Event::MouseWheel(mousex, mousey, (e->wheel.direction == SDL_MOUSEWHEEL_NORMAL ? e->wheel.y : e->wheel.y * -1), 0ul);// *g_Engine->Resources.FrameID);
#ifdef EC_PROFILE
		g_Engine->Send(&ass, g_Engine->Resources.Profile);
#else
		g_Engine->Send(&ass);
#endif // EC_PROFILE
		break;
	}
	case SDL_QUIT:
	{
		Quit = true;
		Quitty();
	}

	}
}

#ifdef EC_PROFILE
#undef PROFILE_PUSH
#undef PROFILE_POP
#define PROFILE_PUSH(x) PROFILE_PUSH_TMP(x)
#define PROFILE_POP() PROFILE_POP_TMP()
#undef PROFILE_PUSH_TMP
#undef PROFILE_POP_TMP
#endif

#ifdef USE_CONSOLE
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
	switch (dwCtrlType) {
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
		Quit();
		return TRUE;

	default:
		return FALSE;
	}
}
#endif
