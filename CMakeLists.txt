# CMakeList.txt : CMake project for Encapsulated, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL") # Make all libraries and crap link to dynamic MSVC runtime (Bullet does this itself, which basically forces this project to as well)

project("Encapsulated")

option(EC_USE_BULLET "Use the Bullet Physics library (Off means no physics)" ON)
option(EC_BUILD_PARALLEL "Build using multiple threads" ON)
option(EC_PROFILE "Build and link with profiling in mind" ON)
option(EC_CONSOLE "Build with console enabled" ON)
option(EC_MAXWARN "Build with highest level of warnings" ON)


# Find Source Files

set(EC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE EC_SRCS "${EC_SRC_DIR}/*.cpp")
file(GLOB_RECURSE EC_HDRS "${EC_SRC_DIR}/*.h")

if (EC_PROFILE)
	add_compile_definitions(EC_PROFILE)
endif()

add_executable(Encapsulated ${EC_SRCS})
set(BUILD_SHARED_LIBS ON)

install(TARGETS Encapsulated
		DESTINATION bin)

install(DIRECTORY GLSL/ DESTINATION bin/Shaders/)

set_target_properties(Encapsulated
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO # Turning Extensions off for increased compatibility - see https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
		$<EC_PROFILE:EXE_LINKER_FLAGS ${CMAKE_LINKER_FLAGS} /Profile>
		) 


if (MSVC)
	if (EC_MAXWARN)
		target_compile_options(Encapsulated PRIVATE /W4)
	endif()
	if(EC_BUILD_PARALLEL)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP5")
	endif()
endif()

target_include_directories(Encapsulated PUBLIC ${EC_SRC_DIR})


# Find Zlib

find_package(ZLIB REQUIRED)
 
target_link_libraries(Encapsulated PRIVATE ${ZLIB_LIBRARIES})
if (BUILD_SHARED_LIBS AND WIN32)
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		install(FILES "${ZLIB_INCLUDE_DIRS}/../bin/zlibd.dll" DESTINATION bin)
	else()
		install(FILES "${ZLIB_INCLUDE_DIRS}/../bin/zlib.dll" DESTINATION bin)
	endif()
endif()


# Find SDL2
find_package(SDL2 QUIET)

if (NOT SDL2_FOUND)
	message("Unable to find SDL2")

	execute_process(COMMAND git submodule update --init -- external/sdl
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

	if(WIN32)
		# if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win64")
			set(LIBC ON CACHE BOOL "Force LibC On" FORCE)
			# add_compile_definitions(HAVE_LIBC=1)
			# message("Setting HAVE_LIBC")
		# endif()
	endif()
	add_subdirectory(external/sdl)
	set(SDL2_LIBRARIES SDL2::SDL2main SDL2::SDL2)
	target_include_directories(Encapsulated PRIVATE $<TARGET_PROPERTY:SDL2::SDL2main,INTERFACE_INCLUDE_DIRECTORIES> $<TARGET_PROPERTY:SDL2::SDL2,INTERFACE_INCLUDE_DIRECTORIES>)
else()
	target_include_directories(Encapsulated PRIVATE SDL2_INCLUDE_DIRECTORIES)
endif()

target_link_libraries(Encapsulated PRIVATE ${SDL2_LIBRARIES})

# Find Assimp
find_package(ASSIMP QUIET)

if (NOT ASSIMP_FOUND)
	message("Unable to find Assimp")
	
	execute_process(COMMAND git submodule update --init -- external/assimp
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	
	# To fix install (assimp has a broken install script with ASSIMP_BUILD_PDB == ON and BUILD_SHARED_LIBS == ON)
	# PDBs are windows specific but this should have no negative effect on other platforms
	set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "Prevent Assimp from installing broken pdb" FORCE)
	add_subdirectory(external/assimp)
	set(ASSIMP_LIBRARIES "assimp")
	target_include_directories(Encapsulated PRIVATE $<TARGET_PROPERTY:assimp,INTERFACE_INCLUDE_DIRECTORIES>)
else()
	target_include_directories(Encapsulated PRIVATE ASSIMP_INCLUDE_DIRECTORIES)
endif()

target_link_libraries(Encapsulated PRIVATE ${ASSIMP_LIBRARIES})


# Find SDL Image

find_package(SDL_IMAGE QUIET)

if (NOT SDL_IMAGE_FOUND)
	message("Did not pre-installed find SDL Image")

	execute_process(COMMAND git submodule update --init -- external/sdl_image
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

	set(PNG_BUILD_ZLIB ON CACHE BOOL "Prevent libpng from building zlib" FORCE)
	add_subdirectory(external/sdl_image)
	set(SDL_IMAGE_LIBRARIES SDL2::image)
	target_include_directories(Encapsulated PRIVATE $<TARGET_PROPERTY:SDL2::image,INTERFACE_INCLUDE_DIRECTORIES>)
else()
	target_include_directories(Encapsulated PRIVATE SDL_IMAGE_INCLUDE_DIRECTORIES)
endif()

target_link_libraries(Encapsulated PRIVATE ${SDL_IMAGE_LIBRARIES})
install(TARGETS SDL2_image
		RUNTIME DESTINATION bin)


# Find SDL TTF

if(WIN32)
	if (NOT ENV{FREETYPE_DIR})
		set(ENV{FREETYPE_DIR} "C:\\Program Files (x86)\\freetype") # I am cheating because at this point I am so damn tired of windows bs
	endif()
endif()


find_package(SDL_TTF QUIET)

if (NOT SDL_TFF_FOUND)
	message("Did not find pre-installed SDL TTF")

	execute_process(COMMAND git submodule update --init -- external/sdl_ttf
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
					
	add_subdirectory(external/sdl_ttf)

	# Fix for windows :(
	if(WIN32 AND BUILD_SHARED_LIBS)
	  target_compile_definitions(SDL2_ttf PRIVATE -DDLL_EXPORT) # Taken from the working SDL_image CMakeLists.txt
	endif()
endif()

target_link_libraries(Encapsulated PRIVATE SDL2_ttf)
target_include_directories(Encapsulated PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL_ttf")


# Find bullet

set(BUILD_BULLET2_DEMOS NO CACHE BOOL "Force bullet not to build demos" FORCE)
set(BUILD_EXTRAS NO CACHE BOOL "Force bullet not to build extras" FORCE)
find_package(BULLET QUIET)

if (NOT BULLET_FOUND)
	message("Did not find pre-installed Bullet")

	execute_process(COMMAND git submodule update --init -- external/bullet3
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

	# On Windows Bullet must be static
	if (WIN32)
		set(EC_TMP_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
		set(BUILD_SHARED_LIBS OFF CACHE BOOL "STATIC BULLET PLZ" FORCE)
	endif()

	set(BUILD_UNIT_TESTS OFF CACHE BOOL "Force Bullet not to build tests (in case they're broken)" FORCE)
	set(INSTALL_LIBS ON CACHE BOOL "Force Bullet to allow installing properly" FORCE)
	add_subdirectory(external/bullet3)

	add_dependencies(Bullet3Geometry Bullet3Common)
	add_dependencies(Bullet3Collision LinearMath Bullet3Common)
	add_dependencies(BulletCollision LinearMath Bullet3Common)
	add_dependencies(Bullet3Dynamics LinearMath Bullet3Common)
	add_dependencies(Bullet2FileLoader LinearMath)

	set(BULLET_LIBRARIES LinearMath BulletCollision BulletDynamics)
	target_include_directories(Encapsulated PRIVATE
		$<TARGET_PROPERTY:BulletCollision,INCLUDE_DIRECTORIES>
		$<TARGET_PROPERTY:BulletDynamics,INCLUDE_DIRECTORIES>
		$<TARGET_PROPERTY:LinearMath,INCLUDE_DIRECTORIES>)

	if (WIN32)
		set(BUILD_SHARED_LIBS ${EC_TMP_BUILD_SHARED_LIBS})
	endif()
else()
	target_include_directories(Encapsulated PRIVATE ${BULLET_INCLUDE_DIRS})
endif()

target_link_libraries(Encapsulated PRIVATE ${BULLET_LIBRARIES})



# Find OpenGL

find_package(OPENGL REQUIRED)

target_link_libraries(Encapsulated PRIVATE ${OPENGL_LIBRARIES})
target_include_directories(Encapsulated PRIVATE ${OPENGL_INCLUDE_DIRECTORIES})

# Find GLEW

find_package(GLEW QUIET)

if (GLEW_FOUND AND ("${GLEW_INCLUDE_DIRECTORIES}" MATCHES "^external/glew^" OR "${GLEW_INCLUDE_DIRS}" STREQUAL ""))
	set(GLEW_FOUND OFF)
	message("Glew was found in the installation directory, ignoring")
endif()

if(NOT GLEW_FOUND)
	message("Did not find glew pre-installed")

	add_subdirectory(external/glew/build/cmake)
	set(GLEW_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/external/glew/include")
	target_link_libraries(Encapsulated PRIVATE glew)
else()
	target_link_libraries(Encapsulated PRIVATE ${GLEW_STATIC_LIBRARIES})
	install(FILES ${GLEW_SHARED_LIBRARIES} DESTINATION bin)
endif()

target_include_directories(Encapsulated PRIVATE ${GLEW_INCLUDE_DIRS})
message("Glew Libraries: ${GLEW_LIBRARIES} --- Glew Include Dir: ${GLEW_INCLUDE_DIRS}")


# Find OpenAL
find_package(OPENAL QUIET)

if(NOT OPENAL_FOUND)
	# couldn't find OpenAL lets use git-submodule instead
	message("Unable to find OpenAL")

	execute_process(COMMAND git submodule update --init -- external/openal-soft
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

	add_subdirectory(external/openal-soft)

	set(OPENAL_LIBRARY OpenAL)
endif()
	
target_link_libraries(Encapsulated PRIVATE ${OPENAL_LIBRARY})