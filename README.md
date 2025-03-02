# cpp-engine

## What is cpp-engine?
`cpp-engine` is a hobby game engine built to learn all that is required to go from words in a cpp file to triangles in a window. It is currently (or foreseeably) not designed for general use; it is very niche and targeted to a specific use.

## Runtime dependencies:
This app requires:
OpenGL 4.5
OpenAL Soft (probably 1.x)
Bullet3 Physics (probably 2.5+)
GLEW (only 2.x has been tested, but 1.x may work as well)
Assimp (any v5)
YAML-CPP (0.7)
On Windows: All except OpenGL (and MSVC) dependencies are packaged alongside the cpp-engine executable.

## Building

### Windows
> Note about WSL: Normally WSL would be an easy way to build and use a linux compatible app, and would make building this engine/game easier.
> However before I realized WSL2 would only support up to OpenGL 4.2, I had already started using OpenGL 4.5 code, and as such unless WSL2 is updated to support OpenGL 4.5, building this app in WSL is possible but will run.

Building on windows is more complex but still possible.
You will have to download and compile the dependencies yourself.
Current dependencies:
Runtime:
OpenGL (This should be built into windows)
SDL2, SDL_image, SDL_ttf, Freetype, OpenAL Soft, Bullet3, zlib, GLEW (These are supplied along with cpp-engine as dlls)
Building:
[SDL2](https://github.com/libsdl-org/SDL)
[SDL_image](https://github.com/libsdl-org/SDL_image)
[SDL_ttf](https://github.com/libsdl-orf/SDL_ttf)
[Freetype](https://gitlab.freedesktop.org/freetype/freetype)
[OpenAL Soft](https://github.com/kcat/openal-soft)
[Bullet3 Physics](](https://github.com/bulletphysics/bullet3)
[zlib](https://github.com/madley/zlib)
[GLEW](https://github.com/nigels-com/glew/releases)

CMake finds these dependencies in the CMakeLists.txt, the strategy for each dependency is as follows:
Build the dependency and install it somewhere onto your system
Each dependency has a `find_package` call in CMakeLists.txt that will look for the installed dependency
Either:
Make the installed dependency location accessible globally (by setting PATH or otherwise)
OR
Append a `-DCMAKE_PREFIX_PATH=C:\Users\me\where\the\dependency\install\is` to the CMake generation/configuration command

Some dependencies may require tweaking the source code to compile and build properly (Freetype may require commenting out the define to use ZLIB).

Once you have all the dependencies downloaded, build and install them somewhere.
Then in the `cpp-engine` directory:
Generate the CMake configuration:
```
cmake -B build -S ./ [-DCMAKE_PREFIX_PATH="C:\Semicolon\Separated;C:\Path\to\install\of\dependencies"]
```
Add the `-DCMAKE_PREFIX_PATH="C:\Semicolon\Separated;C:\Path\to\install\of\dependencies"` option if the install locations (which will be the folders containing the bin/lib/include directories) of the built and installed dependencies are not accessible through the PATH environment variable.

Once CMake has configured the project, you can build and install it:
```
# Build cpp-engine
cmake --build build --config Release

# Install cpp-engine-game and dependencies
cmake --install build --prefix install
```

Once this succeeds, you should fine the `cpp_engine_game.exe` and all dependencies and game assets under `install/bin`.

Congrats! You're done.

### Linux

Ubuntu:
Toolchain dependencies:
```
sudo apt install cmake build-essential
```

cpp-engine dependencies:
```
sudo apt install \
    libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \
    libbullet-dev \
    libopenal-dev \
    libglew-dev \
    libassimp-dev \
    libyaml-cpp-dev
```

Now simple configure, build, and install cpp-engine-game:
```
cd cpp-engine

# Configure the project
cmake -B build -S ./

# Build the project (with 8 threads)
cmake --build build --config Release -j 8

# Install the project
cmake --install build --prefix install
```

Once this succeeds, you should have `cpp_engine_game`, dependencies, and all required game assets under `install/bin`.

Congrats! You're done.