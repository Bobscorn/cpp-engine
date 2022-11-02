cmake_minimum_required(VERSION 3.21)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL") # Make all libraries and crap link to dynamic MSVC runtime (Bullet does this itself, which basically forces this project to as well)
project(cpp_engine_game CXX)

set(CP_ENG_GAME_SRCS 
	"Parkour.cpp"
	"ParkourShapes.cpp"
 "ParkourScene.cpp" "ParkourLevels.cpp"  "ParkourUI.cpp")
   
set(CP_ENGINE_GAME_HDRS 
	"Parkour.h"
	"ParkourShapes.h"
 "ParkourScene.h" "ParkourLevels.h"  "ParkourUI.h")
		
add_executable(cpp_engine_game "Main.cpp" ${CP_ENG_GAME_SRCS} ${CP_ENGINE_GAME_HDRS})
target_link_libraries(cpp_engine_game PRIVATE cpp_engine_lib)
		
set_target_properties(cpp_engine_game
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO # Turning Extensions off for increased compatibility - see https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
		$<EC_PROFILE:EXE_LINKER_FLAGS ${CMAKE_LINKER_FLAGS} /Profile>
		)

if (EC_MAXWARN)
	if (MSVC)
    target_compile_options(cpp_engine_game PRIVATE /W4)
    else()
    target_compile_options(cpp_engine_game PRIVATE -Wall -Wextra -pedantic -Werror)
    endif()
endif()
        
install(TARGETS cpp_engine_game
		DESTINATION bin)

install(FILES $<TARGET_RUNTIME_DLLS:cpp_engine_game>
        DESTINATION bin)

# Example installing other dlls

find_file(ZLIB_DLL_PATH NAMES zlib.dll zlibd.dll zlib1.dll zlib1d.dll PATHS "${CMAKE_PREFIX_PATH}" PATH_SUFFIXES bin lib)
install(FILES ${ZLIB_DLL_PATH} DESTINATION bin OPTIONAL)

find_file(OPENAL_DLL_PATH NAMES OpenAL32.dll PATHS "${CMAKE_PREFIX_PATH}" PATH_SUFFIXES bin lib)
install(FILES ${OPENAL_DLL_PATH} DESTINATION bin OPTIONAL)

find_file(ASSIMP_DLL_PATH NAMES assimp-vc142-mt.dll assimp-vc142-mtd.dll PATHS "${CMAKE_PREFIX_PATH}" PATH_SUFFIXES bin lib)
install(FILES ${ASSIMP_DLL_PATH} DESTINATION bin OPTIONAL)

# End dlls example

install(FILES $<TARGET_PDB_FILE:cpp_engine_game> DESTINATION bin OPTIONAL)
        
install(DIRECTORY "${CMAKE_SOURCE_DIR}/GLSL/" DESTINATION bin/Shaders/)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/Assets/Programs" DESTINATION bin)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/Assets/Materials" DESTINATION bin)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/Assets/Textures" DESTINATION bin)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/Assets/Blocks" DESTINATION bin)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/Assets/Shaders" DESTINATION bin)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/Assets/Fonts" DESTINATION bin)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/Assets/Meshes" DESTINATION bin)

if (NOT TARGET SDL2::SDL2main)
    message(FATAL_ERROR "SDL2::SDL2main is not found. Please install SDL2.")
    return()
endif()

target_link_libraries(cpp_engine_game PRIVATE SDL2::SDL2main)



add_executable(ParkourTests
	"Parkour.cpp"
)

set_target_properties(ParkourTests
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO # Turning Extensions off for increased compatibility - see https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
		) 

target_compile_definitions(ParkourTests PUBLIC CPP_GAME_TESTS=1)

target_include_directories(ParkourTests PRIVATE $<TARGET_PROPERTY:cpp_engine_lib,INCLUDE_DIRECTORIES>)
target_link_libraries(ParkourTests PRIVATE cpp_engine_lib)
target_link_libraries(ParkourTests PRIVATE GTest::gtest_main GTest::gmock)

add_custom_command(TARGET ParkourTests POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy
						$<TARGET_RUNTIME_DLLS:ParkourTests>
						$<TARGET_FILE_DIR:ParkourTests>
					COMMAND_EXPAND_LISTS)

include(GoogleTest)
gtest_discover_tests(ParkourTests)



get_target_property(CP_ENG_GAME_DEPS cpp_engine_game LINK_LIBRARIES)
message("cpp_engine_game dependencies: ${CP_ENG_GAME_DEPS}")

