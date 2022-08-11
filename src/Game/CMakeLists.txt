cmake_minimum_required(VERSION 3.8)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL") # Make all libraries and crap link to dynamic MSVC runtime (Bullet does this itself, which basically forces this project to as well)
project(cpp_engine_game CXX)

set(CP_ENG_GAME_SRCS "Main.cpp")
set(CP_ENGINE_GAME_HDRS "")
		
add_executable(cpp_engine_game ${CP_ENG_GAME_SRCS})
target_link_libraries(cpp_engine_game PRIVATE cpp_engine_lib)
		
set_target_properties(cpp_engine_game
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO # Turning Extensions off for increased compatibility - see https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
		$<EC_PROFILE:EXE_LINKER_FLAGS ${CMAKE_LINKER_FLAGS} /Profile>
		)

install(TARGETS cpp_engine_game
		DESTINATION bin)
        
install(DIRECTORY "${CMAKE_SOURCE_DIR}/GLSL/" DESTINATION bin/Shaders/)

if (NOT TARGET SDL2::SDL2main)
    message(FATAL_ERROR "SDL2::SDL2main is not found. Please install SDL2.")
    return()
endif()

target_link_libraries(cpp_engine_game PRIVATE SDL2::SDL2main)