# cpp-engine

## Requirements

If building with Audio, use option EC_BUILD_AUDIO
You will need to put OpenAL32.lib into <repo dir>/Libraries/OpenAL

If building with Physics (recommended), use option EC_USE_BULLET
You will need to put BulletCollision.lib, BulletDynamics.lib and LinearMath.lib into <repo dir>/Libraries/Bullet


### Libraries

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
Debug Build Binaries:
- OpenAL32.lib
- ex-common.lib

You will likely have to manually build OpenAL

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

#### OpenGL/GLEW
Place Libraries/Binaries in Folder: <repo dir>/Libraries/OpenGL
Release Build Binaries:
- glew32.lib
- glew32s.lib
Debug Build Binaries:
- glew32d.lib
- glew32sd.lib

You will almost definitely download precompiled Glew binaries

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