# CMakeList.txt : CMake project for Encapsulated, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project("Encapsulated")

set(EC_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(EC_LIBRARY_DIR "${EC_BASE_DIR}/Libraries")
set(SDL_DIR "${EC_LIBRARY_DIR}/SDL")
set(SDL_IMAGE_DIR "${EC_LIBRARY_DIR}/SDLImage")
set(SDL_TTF_DIR "${EC_LIBRARY_DIR}/SDLTTF")
set(BULLET_DIR "${EC_LIBRARY_DIR}/Bullet3")
set(GLEW_DIR "${EC_LIBRARY_DIR}/Glew")
set(ASSIMP_DIR "${EC_LIBRARY_DIR}/Assimp")

# Include Directories
cmake_policy(SET CMP0077 NEW)

option(EC_USE_BULLET "Use the Bullet Physics library (Off means no physics)" ON)
option(EC_BUILD_PARALLEL "Build using multiple threads" ON)
option(EC_PROFILE "Build and link with profiling in mind" ON)
option(EC_CONSOLE "Build with console enabled" ON)

if (EC_PROFILE)
	if (MSVC)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /Profile")
	endif (MSVC)
	add_definitions(-DEC_PROFILE)
	message("Enabled Windows Profiling")
endif (EC_PROFILE)

if(EC_USE_BULLET)
add_definitions(-DUSE_BULLET_STUFF)
endif(EC_USE_BULLET)

option(SDL_LOAD_PNG "Link in PNG libraries and tell SDL to load PNG support" ON)
option(SDL_LOAD_JPEG "Link in JPEG libraries and tell SDL to load JPEG support" ON)
option(SDL_LOAD_TIFF "Link in TIFF libraries and tell SDL to load TIFF support" OFF)
option(SDL_LOAD_WEBP "Link in WEBP libraries and tell SDL to load WEBP support" OFF)
option(EC_BUILD_AUDIO "Link and build with OpenAL Libraries for Audio support" ON)

if(SDL_LOAD_PNG)
	add_definitions(-DEC_LOAD_PNG)
endif(SDL_LOAD_PNG)
if(SDL_LOAD_JPEG)
	add_definitions(-DEC_LOAD_JPEG)
endif(SDL_LOAD_JPEG)
if(SDL_LOAD_TIFF)
	add_definitions(-DEC_LOAD_TIFF)
endif(SDL_LOAD_TIFF)
if(SDL_LOAD_WEBP)
	add_definitions(-DEC_LOAD_WEBP)
endif(SDL_LOAD_WEBP)

if (WIN32)
	# Windows
	add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)

	# Include Dirs
	set(INCLUDE_DIRS ${INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
	set(INCLUDE_DIRS ${INCLUDE_DIRS} "${SDL_DIR}/include")
	set(INCLUDE_DIRS ${INCLUDE_DIRS} "${SDL_IMAGE_DIR}/include")
	set(INCLUDE_DIRS ${INCLUDE_DIRS} "${SDL_TTF_DIR}/include")
	set(INCLUDE_DIRS ${INCLUDE_DIRS} "${BULLET_DIR}/src")
	set(INCLUDE_DIRS ${INCLUDE_DIRS} "${GLEW_DIR}/include")
	set(INCLUDE_DIRS ${INCLUDE_DIRS} "${ASSIMP_DIR}/include")
	if (EC_BUILD_AUDIO)
		set(EC_OPENAL_LOCATION "C:/Users/joshu/Documents/tmp/openal-soft-1.21.1")
		set(INCLUDE_DIRS ${INCLUDE_DIRS} "${EC_OPENAL_LOCATION}/include")
	else()
		add_definitions(-DEC_NO_AUDIO)
	endif(EC_BUILD_AUDIO)

	# Headers	
	file(GLOB SDL_HDRS "${SDL_DIR}/include")
	set(SDL_HDRS ${SDL_HDRS} "${SDL_IMAGE_DIR}/include/SDL_image.h")
	set(EC_HDRS ${EC_HDRS} ${SDL_HDRS})
	
	# Build time reduction (linux parallel done in build shell script)

	if (EC_BUILD_PARALLEL)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP5")
	endif(EC_BUILD_PARALLEL)

	# Libraries
	if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		set(EC_LIBRARIES ${EC_LIBRARIES} "${SDL_DIR}/VisualC/x64/Release/SDL2.lib")
		set(EC_LIBRARIES ${EC_LIBRARIES} "${SDL_DIR}/VisualC/x64/Release/SDL2main.lib")
		set(EC_LIBRARIES ${EC_LIBRARIES} "${BULLET_DIR}/build_cmake/lib/Release/BulletCollision.lib")
		set(EC_LIBRARIES ${EC_LIBRARIES} "${BULLET_DIR}/build_cmake/lib/Release/BulletDynamics.lib")
		set(EC_LIBRARIES ${EC_LIBRARIES} "${BULLET_DIR}/build_cmake/lib/Release/LinearMath.lib")
		set(EC_LIBRARIES ${EC_LIBRARIES} "${ASSIMP_DIR}/build/code/Release/assimp-vc140-mt.lib")
		set(EC_LIBRARIES ${EC_LIBRARIES} ${CMAKE_CURRENT_SOURCE_DIR}/lib/Release/glew32.lib ${CMAKE_CURRENT_SOURCE_DIR}/lib/Release/glew32s.lib)
		if (EC_BUILD_AUDIO)
			set(EC_LIBRARIES ${EC_LIBRARIES} "${EC_OPENAL_LOCATION}/build/Release/OpenAL32.lib")
			set(EC_LIBRARIES ${EC_LIBRARIES} "${EC_OPENAL_LOCATION}/build/Release/ex-common.lib")
		endif(EC_BUILD_AUDIO)
		message("Building Release")
	else()
		set(EC_LIBRARIES ${EC_LIBRARIES} "${SDL_DIR}/VisualC/x64/Debug/SDL2.lib")
		set(EC_LIBRARIES ${EC_LIBRARIES} "${SDL_DIR}/VisualC/x64/Debug/SDL2main.lib")
		set(EC_LIBRARIES ${EC_LIBRARIES} "${BULLET_DIR}/build_cmake/lib/Debug/BulletCollision_Debug.lib")
		set(EC_LIBRARIES ${EC_LIBRARIES} "${BULLET_DIR}/build_cmake/lib/Debug/BulletDynamics_Debug.lib")
		set(EC_LIBRARIES ${EC_LIBRARIES} "${BULLET_DIR}/build_cmake/lib/Debug/LinearMath_Debug.lib")
		set(EC_LIBRARIES ${EC_LIBRARIES} "${ASSIMP_DIR}/build/code/Debug/assimp-vc140-mt.lib")
		set(EC_LIBRARIES ${EC_LIBRARIES} ${CMAKE_CURRENT_SOURCE_DIR}/lib/Debug/glew32d.lib ${CMAKE_CURRENT_SOURCE_DIR}/lib/Debug/glew32sd.lib)
		if (EC_BUILD_AUDIO)
			set(EC_LIBRARIES ${EC_LIBRARIES} "${EC_OPENAL_LOCATION}/build/Debug/OpenAL32.lib")
			set(EC_LIBRARIES ${EC_LIBRARIES} "${EC_OPENAL_LOCATION}/build/Debug/ex-common.lib")
		else()
			add_definitions(-DEC_NO_AUDIO)
		endif(EC_BUILD_AUDIO)
		message("Building Debug")
	endif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		
	set(WTFWHOUSESTHISLONGAVARIABLE ${QUICK})
	set(QUICK "${CMAKE_CURRENT_SOURCE_DIR}/lib")
	set(EC_LIBRARIES ${EC_LIBRARIES} Opengl32 glu32)
	set(EC_LIBRARIES ${EC_LIBRARIES} ${QUICK}/SDL2_image.lib ${QUICK}/SDL2_ttf.lib)
	set(QUICK ${WTFWHOUSESTHISLONGAVARIABLE})

	message("Libraries are: ${EC_LIBRARIES}")
else()
	# Linux
	set(INCLUDE_DIRS ${INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}")
	set(BULLET_INCLUDE_DIRECTORY "${BULLET_DIR}/src")
	set(INCLUDE_DIRS ${INCLUDE_DIRS} ${BULLET_INCLUDE_DIRECTORY})
	# Bullet include directories have not been completed
	set(INCLUDE_DIRS ${INCLUDE_DIRS} "${ASSIMP_DIR}/include")

	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_definitions(-D_DEBUG)
	endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

	set(EC_LIBRARIES ${EC_LIBRARIES} "${BULLET_DIR}/build/src/BulletDynamics/libBulletDynamics.a" "${BULLET_DIR}/build/src/BulletCollision/libBulletCollision.a" rt "${BULLET_DIR}/build/src/LinearMath/libLinearMath.a")
	set(EC_LIBRARIES ${EC_LIBRARIES} "${ASSIMP_DIR}/lib/libassimp.so" "${ASSIMP_DIR}/lib/libIrrXML.a")
	
	# Headers
	file(GLOB SDL_HDRS ${SDL_INCLUDE_DIRECTORY})
	
	set(EC_LIBRARIES ${EC_LIBRARIES} SDL2 SDL2main SDL2_image SDL2_ttf dl pthread sndio freetype png z jpeg asound wayland-client GL GLU GLEW)
	set(EC_LIBRARIES ${EC_LIBRARIES} stdc++fs)
endif(WIN32)

# Encapsulated Code files

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE EC_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE EC_HDRS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

if (WIN32)
	# Natvis
	set(EC_SRCS ${EC_SRCS} "${CMAKE_CURRENT_SOURCE_DIR}/Helpers/btAlignedObjectArray.natvis")
endif(WIN32)

add_executable (Encapsulated ${EC_SRCS} ${EC_HDRS})

option(WARNINGS_AS_ERRORS "Turn on maximum warning level, and treat all warnings as errors" OFF)

if(WARNINGS_AS_ERRORS)
	if(MSVC)
		target_compile_options(Encapsulated PRIVATE /W4 /WX)
	else(MSVC)
		target_compile_options(Encapsulated PRIVATE -Wall -Wextra -pedantic -Werror)
	endif(MSVC)
endif(WARNINGS_AS_ERRORS)

if (NOT EC_CONSOLE)
	if (MSVC)	
		set_target_properties(Encapsulated PROPERTIES LINK_FLAGS "/SUBSYSTEM:windows")
	endif(MSVC)
endif(NOT EC_CONSOLE)

target_include_directories(Encapsulated PRIVATE ${INCLUDE_DIRS})
target_link_libraries(Encapsulated ${EC_LIBRARIES})
