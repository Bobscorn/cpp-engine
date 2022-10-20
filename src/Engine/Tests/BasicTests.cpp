

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

#include <gtest/gtest.h>

TEST(BasicTests, CanPrint)
{
	std::cout << "Test Print" << std::endl;
}

TEST(BasicTests, CanCreateFile)
{
	std::ofstream file("test.txt", std::ios::trunc);
	ASSERT_TRUE(file.good());
	ASSERT_TRUE(file.is_open());
	file << "Test File Writing" << std::endl;
	ASSERT_TRUE(file.good());
}

TEST(BasicTests, SDL_Init_Test)
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

	ASSERT_EQ(sdlInitResult, 0);
	
	SDL_Quit();
}

#endif