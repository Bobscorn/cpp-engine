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

(As of writing this is, this define is at line 198 in ftoption.h)

Then to install:
Run the following in the root directory of the modified freetype clone:

	cmake -B build
	cmake --build build --config Release
	cmake --install build --config Release


And lastly to fix a broken assimp cmake script if using the assimp clone, remove the INSTALL lines 479 and 480 from the assimp CMakeLists.txt.

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
Probably, it could in fact be much simpler to run a prebuilt client on linux with much less than these packages.
Especially if static linking is used.
Also chances are several libraries such as libgl, libpng, libz, libglu, ligdl etc. are pre-installed.

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















### New library requirements

Libraries moved outside of source.

So clone:
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
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- (gzip?)


We now build each individual dependency on its own (we used to build them as subdirectories before).
To make sure it uses msvc (windows), you may wish to use the powershell/cmd for vs2022 (or vs any version) (Developer Powershell for VS 2022 is one of the names).
This will help cmake use and find the correct compiler.

Otherwise for mingw, specify either the "MinGW Makefiles" generator for cmake, or pass -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ to the cmake generation command, and make sure gcc and g++ are added to path.
Make sure you compile each library with the same compiler and options.

For building All of the packages, you may use a different generator or compiler, and may build and install them to different places.
For consistency these steps use the Ninja generator, inside a Developer Powershell for VS 2022, where cmake chooses the msvc compiler.
These steps should be reproducible with MinGW or any other compiler, simply specify `-DCMAKE_C_COMPILER=<path to C compiler> -DCMAKE_CXX_COMPILER=<path to c++ compiler>` at config time.
These steps place each package's build folder in the same location as the source folder in build/.
For installation these steps place each package's install files into `<package dir>/../install`.
This puts all package install files into the same folder.
These steps assume you have cloned all packages into the same directory.

You may also explicitly specify building in Release mode by adding `-DCMAKE_BUILD_TYPE=Release` to the cmake configure steps.
Or with multi-config generators, you may simply specify at build time which configuration to build (--config Debug/--config Release).
Example explicitly setting the build type at config time:

    cmake -B build -DCMAKE_BUILD_TYPE=Release
    
Example explicitly setting the build type at build time:
    
    <run cmake config step>
    cmake --build build --config Release

Ensure that you compile all packages with the same compiler, built for the same architecture. Mixing debug and release should not cause problems, but may do so anyway.
Also ensure that you properly specify correct paths when building packages that depend on one another, and give cpp-engine the correct paths to the libraries.


## Per Package building
    
For building bullet:
    Use regular cmake syntax,
    It must build at least the Release build, (Debug can be added but the find_package will fail for bullet if there isn't release libraries)
    It must have -DINSTALL_LIBS=ON to install properly
    It must have -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON on windows (unless you are building everything to use static msvc runtime)
    
    cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DINSTALL_LIBS=ON -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON
    cmake --build build
    cmake --install build --prefix ../install

For building ZLIB:
    Use regular cmake syntax, also specifying at config time the install prefix:
    
    cmake -B build -G "Ninja" -DCMAKE_INSTALL_PREFIX="../install"
    cmake --build build
    cmake --install build

For building SDL2:
	Use regular cmake syntax:
	
    cmake -B build -G "Ninja"
    cmake --build build
    cmake --install build --prefix "../install"

For building SDL_image:
	Then use regular cmake syntax, specifying the install prefix at config time, and the CMAKE_PREFIX_PATH:
    
    cmake -B build -G "Ninja" -DCMAKE_PREFIX_PATH="../install" -DCMAKE_INSTALL_PREFIX="../install" 
    cmake --build build
    cmake --install build

For building freetype:
    Use regular cmake syntax with CMAKE_PREFIX_PATH:
    
    cmake -B build -G "Ninja" -DCMAKE_PREFIX_PATH="../install"
    cmake --build build
    cmake --install build --prefix ../install
    
For building SDL_ttf
    Use regular cmake syntax with CMAKE_PREFIX_PATH and BUILD_SHARED_LIBS:
    
    cmake -B build -G "Ninja" -DCMAKE_PREFIX_PATH="../install" -DBUILD_SHARED_LIBS=ON
    cmake --build build
    cmake --install build --prefix ../install

For building Assimp:
Use regular cmake syntax:
However when installing, at least on windows you will have to move the assimp-vc142-mtd.pdb (or whatever file the script complains about) file from <build dir>/bin/ to <build dir>/code/
    
    cmake -B build -G "Ninja"
    cmake --build build
    # example copying
    cp build/bin/assimp-vc142-mtd.pdb build/code/assimp-vc
    cmake --install build --prefix ../install
    
For building open-al:
Use regular cmake syntax:

        cmake -B build -G "Ninja"
        cmake --build build
        cmake --install build --prefix ../install
        
For building glew:
Use regular cmake syntax:
Except the cmakelists is inside <glew dir>/build/cmake/ rather than directly inside <glew dir>/

    cd build/cmake/
    cmake -B build -G "Ninja"
    cmake --build build
    cmake --install build --prefix ../../../install

For building yaml-cpp:
Use regular cmake syntax also specifying BUILD_SHARED_LIBS:

        cmake -B build -G "Ninja" -DBUILD_SHARED_LIBS=ON
        cmake --build build
        cmake --install build --prefix ../install

And that's it.

To now build with these dependancies, if you have installed all packages into <cpp-engine-dir>/../install/ you can build the engine by simply running:
    
    cmake -B build -G "Ninja" -DCMAKE_PREFIX_PATH="../install"
    cmake --build build
    cmake --install build
    
Or you can leave it blank, it will default to "../install" if not explicitly set.
If you have installed packages to another directory, you will have to adjust the CMAKE_PREFIX_PATH accordingly.
I believe (but have not tested) you can define the CMAKE_PREFIX_PATH as a list of paths using semicolons as separators.
This means if you have installed packages into more than one directory, you can specify these directories as well.
Otherwise for platforms like linux, you will likely not have to build these yourself, and can instead install them using your package manager.
CMake should find installed packages as per normal.
    