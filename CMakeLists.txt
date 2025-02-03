cmake_minimum_required(VERSION 3.21)

# Make sure all projects use the same C Runtime
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL") # Make all libraries and crap link to dynamic MSVC runtime (Bullet does this itself, which basically forces this project to as well)
#set(CMAKE_CXX_COMPILER_WORKING 1)

project(cpp_engine CXX)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG c29315dda476f195298ab8da180e564478649b9e
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

option(EC_USE_BULLET "Use the Bullet Physics library (Off means no physics)" ON)
option(EC_BUILD_PARALLEL "Build using multiple threads" OFF)
option(EC_PROFILE "Build and link with profiling in mind" OFF)
option(EC_CONSOLE "Build with console enabled" ON)
option(EC_MAXWARN "Build with highest level of warnings" ON)
option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)

# new stuff v

if (NOT CMAKE_PREFIX_PATH)
    get_filename_component(CP_ENG_INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/../install" ABSOLUTE)
    get_filename_component(CP_ENG_INSTALL_RELEASE_DIR "${CMAKE_CURRENT_LIST_DIR}/../install" ABSOLUTE)
    set(CMAKE_PREFIX_PATH "${CP_ENG_INSTALL_RELEASE_DIR};${CP_ENG_INSTALL_DIR}")
endif()

find_package(ZLIB REQUIRED)
message("- Found ZLIB: ${ZLIB_VERSION}")

find_package(SDL2 REQUIRED)
message("- Found SDL2: ${SDL2_VERSION}")
message("- SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
message("- SDL2_INCLUDE_DIR: ${SDL2_INCLUDE_DIRS}")

find_package(SDL2_ttf REQUIRED)
message("- Found SDL2_ttf: ${SDL_TTF_VERSION}")
set(SDL_TTF_LIBRARIES "SDL2_ttf::SDL2_ttf")
set(SDL_TTF_INCLUDE_DIRS "$<TARGET_PROPERTY:SDL2_ttf::SDL2_ttf,INTERFACE_INCLUDE_DIRECTORIES>")
message("- SDL_ttf_LIBRARIES: ${SDL_TTF_LIBRARIES}")
message("- SDL_TTF_INCLUDE_DIRS: ${SDL_TTF_INCLUDE_DIRS}")

# list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(SDL2_image REQUIRED)
message("- Found SDL2_image: ${SDL2_image_VERSION}")
set(SDL2_IMAGE_LIBRARIES "SDL2_image::SDL2_image$<$<BOOL:${BUILD_SHARED_LIBS}>:-static>")
set(SDL2_IMAGE_INCLUDE_DIRS "$<TARGET_PROPERTY:SDL2_image::SDL2_image,INTERFACE_INCLUDE_DIRECTORIES>")
message("- SDL2_image_INCLUDE_DIRS: ${SDL2_IMAGE_INCLUDE_DIRS}")
message("- SDL2_image_LIBRARIES: ${SDL2_IMAGE_LIBRARIES}")

set(BULLET_ROOT ${CMAKE_PREFIX_PATH})
find_package(Bullet REQUIRED) # Prevent Cmake from using an installed FindBullet.cmake file (which doesn't seem to work with a prebuilt one)
message("- Found Bullet: ${BULLET_VERSION}")
message("- Bullet_INCLUDE_DIRS: ${BULLET_INCLUDE_DIRS}")
message("- Bullet_LIBRARIES: ${BULLET_LIBRARIES}")

find_package(OpenAL REQUIRED)
message("- Found OpenAL: ${OpenAL_VERSION}")
message("- OpenAL_INCLUDE_DIRS: ${OPENAL_INCLUDE_DIR}")
message("- OpenAL_LIBRARY: ${OPENAL_LIBRARY}")

find_package(GLEW REQUIRED)
message("- Found GLEW: ${GLEW_VERSION}")
set(GLEW_LIBRARIES "GLEW::GLEW")
set(GLEW_INCLUDE_DIRS "$<TARGET_PROPERTY:GLEW::GLEW,INTERFACE_INCLUDE_DIRECTORIES>")
message("- GLEW_LIBRARIES: ${GLEW_LIBRARIES}")
message("- GLEW_INCLUDE_DIRS: ${GLEW_INCLUDE_DIRS}")

find_package(OpenGL REQUIRED)
message("- Found OpenGL: ${OPENGL_VERSION}")
message("- OpenGL_LIBRARIES: ${OPENGL_LIBRARIES}")
message("- OpenGL include dir: ${OPENGL_INCLUDE_DIR}")

find_package(YAML-CPP REQUIRED)
message("- Found YAML-CPP: ${YAML_CPP_VERSION}")
message("- YAML_CPP_INCLUDE_DIR: ${YAML_CPP_INCLUDE_DIR}")
message("- YAML_CPP_LIBRARIES: ${YAML_CPP_LIBRARIES}")

find_package(assimp REQUIRED)
message("- Found Assimp: ${ASSIMP_VERSION}")
message("- ASSIMP_INCLUDE_DIR: ${ASSIMP_INCLUDE_DIRS}")
message("- ASSIMP_LIBRARIES: ${ASSIMP_LIBRARIES}")


enable_testing()

add_subdirectory(src/Engine)
add_subdirectory(src/Game)

message("CXX Flags: ${CMAKE_CXX_FLAGS}")
