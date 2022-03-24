# cpp-engine

## Requirements

If building with Audio, use option EC_BUILD_AUDIO
You will need to put OpenAL32.lib into <repo dir>/Libraries/OpenAL

If building with Physics (recommended), use option EC_USE_BULLET
You will need to put BulletCollision.lib, BulletDynamics.lib and LinearMath.lib into <repo dir>/Libraries/Bullet
Building without physics has been deprecated.

For Windows, The Cmake Config should automatically download any libraries you don't have,
With the exception of zlib, glew and freetype (a dependency of SDL_ttf)

Please download zlib from https://github.com/madley/zlib and a glew release from https://github.com/nigels-com/glew/releases (for glew get the source code not the binaries) (linux might not have to)

To build and install Debug and Release zlib builds Run:

	cmake -B build
	cmake --build build
	cmake --install build --config Debug
	cmake --install build --config Release

To get glew working, unpack the contents of the glew release zip file into <repo dir>/external/glew such that the glew README.md is at <repo dir>/external/glew/README.md
Cmake should find it from there

To build and install freetype, use similar instructions to zlib,
However on windows (linux hasn't been tested and probably 'just works') you will have to go into the repo, go into <repo dir>/include/freetype/config/ftoption.h, 
And comment out 

	//#define FT_CONFIG_OPTION_USE_ZLIB

(As of writing this is line 198 in ftoption.h)
Then to install:
Run the following in the root directory of the modified freetype clone:

	cmake -B build
	cmake --build build --config Release
	cmake --install build --config Release


This is most likely only a windows requirement, linux usually has prebuilt versions of all these dependencies as packages.
On Windows, this will build and install zlib and freetype binaries and include headers into your C:\Program Files (x86)\ folder.
Currently there is a singular cmake hack that allows SDL_ttf to find freetype without trouble by setting the environment variable FREETYPE_DIR (done with set(ENV{FREETYPE_DIR} "C:\\Program Files (x86)\\freetype)),
To Change this hardcoding, define the environment variable yourself to wherever you have installed freetype before running cmake.

Every other dependency is automatically managed by cmake and git submodules (likely on linux cmake will look for packages, so you may install those)
The dependencies are:
- [SDL2](https://github.com/libsdl-org/SDL)
- [SDL_image](https://github.com/libsdl-org/SDL_image)
- [SDL_ttf](https://github.com/libsdl-orf/SDL_ttf)
	- [Freetype](https://gitlab.freedesktop.org/freetype/freetype)
- [OpenAL](https://github.com/kcat/openal-soft)
- [Bullet Physics](https://github.com/bulletphysics/bullet3)
- [zlib](https://github.com/madley/zlib)
- [Assimp](https://github.com/assimp/assimp)
- OpenGL/glu
- [GLEW](https://github.com/nigels-com/glew)
- There will also probably be SDL_net in the future

Cmake will look for these packages itself, and if it does not find them,
It will clone the github repo into a submodule and Cmake will build it for you.
It does this for all dependencies EXCEPT OpenGL, GLU, zlib and freetype

This means if you use cmake to install these packages yourself, cmake won't download the repos.

Any repos it does download will appear in the external/ folder.


### Libraries

--- OLD LIBRARIES ---

 LINUX REQUIREMENTS HAVE NOT BEEN TESTED
 THESE ARE MERELY GUESSES BASED ON PREVIOUS LINUX BUILDS

Library Requirements for Building on Linux are:
- libsdl2-dev
- libsdl2-image-dev
- libsdl2-ttf-dev
- libdl
- libpthread
- libsndio
- libfreetype
- libpng
- libz
- libjpeg
- libasound
- libwayland-client
- libgl
- libglu
- libglew

You will also need to build Bullet physics, and put the libBulletDynamics.a, libBulletCollision.a and libLinearMath.a into <repo dir>/Libraries/Bullet

Package requirements for running on Linux are:
- libsdl2-2.0-0
- libsdl2-image-2.0-0
- libsdl2-ttf-2.0-0
- libdl
- libpthread
- libsndio
- libfreetype
- libpng
- libz
- libjpeg
- libasound
- libwayland-client
- libgl
- libglu
- libglew
Probably, it could in fact be much simpler to run a prebuilt client on linux with much less than these packages

A prebuilt linux client does not need any Bullet libraries



### Windows Libraries

The following Packages have first: 
- a Library Folder (something like <repo dir>/Libraries/OpenAL) where cpp-engine expects the libraries to be stored
- Expected names of compiled or downloaded binaries to be put into the Library Folder

Some packages have separate Debug and Release binaries, it is safe to put both sets of binaries into the same folder

#### OpenAL:
Place Libraries/Binaries in Folder: <repo dir>/Libraries/OpenAL
Release Build Binaries:
- OpenAL32.lib
- ex-common.lib
Debug Build Binaries: (you might have to append _Debug to their names)
- OpenAL32_Debug.lib
- ex-common_Debug.lib

You will likely have to manually build OpenAL

#### SDL2
Place Libraries/Binaries in Folder: <repo dir>/Libraries/SDL2
Release Build Binaries:
- 

#### Bullet Physics
Place Libraries/Binaries in Folder: <repo dir>/Libraries/Bullet
Release Build Binaries:
- BulletCollision.lib
- BulletDynamics.lib
- LinearMath.lib
Debug Build Binaries:
- BulletCollision_Debug.lib
- BulletDynamics_Debug.lib
- LinearMath_Debug.lib

You will likely have to manually build Bullet Physics

#### Assimp
Place Libraries/Binaries in Folder: <repo dir>/Libraries/Assimp
Build Binaries:
- assimp-vc140-mt.lib

You will likely have to manually build Assimp

#### OpenGL/GLU/GLEW
Place Libraries/Binaries in Folder: <repo dir>/Libraries/OpenGL
Release Build Binaries:
- glew32.lib
- glew32s.lib
Debug Build Binaries:
- glew32d.lib
- glew32sd.lib

Opengl32 and glu32 are also linked against, but should come with windows itself (I think)

You will almost definitely download precompiled OpenGL and glu binaries
You can build Glew binaries yourself

## Building

Once all libraries have been placed into their folders, use standard cmake to generate project files

eg.

`cmake -B build -S ./`

If you're on windows you'll likely open the solution file in <repo dir>/build in Visual Studio and build it that way
Alternatively you can use MSBuild to build from command line

Example usage if you have added msbuild to path:

`cmake -B build -S .\`
`msbuild build\Encapsulated.sln`

If you're on Linux you'll likely just run `make` in the build folder

eg.

`cmake -B build -S ./`
`cd build`
`make`

You should see an `Encapsulated` or `Encapsulated.out`

## Running

Once you have built the Encapsulated executable, for linux users it should be as straight forward as running the Encapsulated executable file
There are a few command line options, but I've forgotten them all and will update this when I remember them.

On windows however, there may be an additional step, depends on whether I get cmake to do this last part automatically

On windows, not all libraries are able to be linked at compile time, and you will require a few shared libraries to be in the executable's folder
<br>
The expected dll files are as follows:
- assimp-vc140-mt.dll
- glew32d.dll (might be a debug only dll)
- libfreetype-6.dll
- libjpeg-9.dll
- libpng16-16.dll
- libtiff-5.dll
- libwebp-7.dll
- SDL2.dll
- SDL2_image.dll
- SDL2_ttf.dll
- zlib1.dll

Once you have those, cpp-engine should run just fine.

### Config

cpp-engine does use a config file, the file is something like <executable directory>/Config/window-config.xml 
Currently this just saves the last position, size, and fullscreen state of the application's last session
Deleting it is perfectly fine
There is currently no option to prevent cpp-engine to not create this file