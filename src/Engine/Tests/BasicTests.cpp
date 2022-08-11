

#ifdef CPP_ENGINE_TESTS

#include <iostream>
#include <fstream>

#ifdef __linux__
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#else
#include <SDL.h>
#include <SDL_image.h>
#endif

#include "TestHelper.h"

int CanPrint();
int CanCreateFile();

int SDL_Init_Test();


int BasicTests(int argc, char* argv[])
{
	std::cout << "Basic Tests" << std::endl;
	TRY_TEST(CanPrint);
	TRY_TEST(CanCreateFile);
	TRY_TEST(SDL_Init_Test);

	return 0;
}


int CanPrint()
{
	std::cout << "Test Print" << std::endl;
	return 0;
}

int CanCreateFile()
{
	std::ofstream file("test.txt", std::ios::trunc);
	if (!file.good())
		return 1;
	if (!file.is_open())
		return 2;
	file << "Test File Writing" << std::endl;
	return 0;
}

int SDL_Init_Test()
{
	struct SDLInitStruct
	{
		int result;

		SDLInitStruct(Uint32 flags)
			: result(SDL_Init(flags))
		{
		}

		~SDLInitStruct()
		{
			SDL_Quit();
		}
		
		inline operator int() const { return result; }
	};

	Uint32 flags = 0;

	auto sdlInitResult = SDL_Init(flags);

	TEST_ASSERT(sdlInitResult == 0);

	return 0;
}

#endif