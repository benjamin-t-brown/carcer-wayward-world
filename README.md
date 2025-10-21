# VortexXe

Deluxe version!

The web version of this game is available here: https://benjamin-t-brown.github.io/vortexxe/

A twist on the arcade game Asteroids, inspired by the old mac game [Maelstrom](https://www.libsdl.org/projects/Maelstrom/).

----
![image](https://github.com/user-attachments/assets/0fe5685a-ede7-48f9-942a-01524bb2b672)
----
![image](https://github.com/user-attachments/assets/ab793726-ff7d-4345-9e7d-d7c44be68504)
----

Keyboard controls: arrow keys left/right: move ship, arrow key up: boost, space: shoot, left shift: shield

Gamepad controls:
    Direction Pad Left/Right: Rotate Ship<br>
    Direction Pad Down: Switch Weapon<br>
    A: Fire<br>
    B or Direction Pad Up: Accelerate<br>
    Y: Shield<br>

## Development

Development Requirements

- GCC/Clang
- SDL2
- SDL2_image
- SDL2_ttf
- SDL2_mixer

For Building WASM Executable

- Emscripten

For Building MiyooA30 Executable

- Docker

This program is built with the Makefile in the src directory.

```
cd src
make -j8
make run
```

### Ubuntu

```
apt install\
 build-essential\
 make\
 clangd-17\
 clang-format\
 libsdl2-ttf-dev\
 libsdl2-image-dev\
 libsdl2-mixer-dev\
 libsdl2-gfx-dev -y
```

### Mac M1^ (brew)

```
brew install gcc@13
brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf

# additional development tools
python3 -m pip install --upgrade setuptools
python3 -m pip install --upgrade pip
```

### Winget (Windows)

If you don't have a windows gcc env, here's way to set it up with some powershell scripts:

https://git.learnjsthehardway.com/learn-code-the-hard-way/lcthw-windows-installers

```
# When installing WinLibs, this takes a bit of time with no feedback, like 30 mins or so, just trust it works eventually
irm https://git.learnjsthehardway.com/learn-code-the-hard-way/lcthw-windows-installers/raw/branch/master/base.ps1 -outfile base.ps1
powershell -executionpolicy bypass .\base.ps1

# Update path, then open a new shell
irm https://git.learnjsthehardway.com/learn-code-the-hard-way/lcthw-windows-installers/raw/branch/master/pathfixer.ps1 -outfile pathfixer.ps1
powershell -executionpolicy bypass .\pathfixer.ps

# Install cpp tools
irm https://git.learnjsthehardway.com/learn-code-the-hard-way/lcthw-windows-installers/raw/branch/master/python.ps1 -outfile cpp.ps1
powershell -executionpolicy bypass .\cpp.ps1

# Install sdl2
powershell -executionpolicy bypass scripts\install_sdl.ps1
```

I prefer to use the git bash shell from here, since it has linux tools, so I add the winlibs path to the .bashrc
`APPDATA\Local\Programs\WinLibs\mingw64`

### Ucrt64/Mingw64 via MSYS2 (Windows)

Assuming you're using ucrt64 (other distribs like mingw64 or clang should work fine):

```
pacman -S base-devel\
 gcc\
 ucrt64/mingw-w64-x86_64-SDL2\
 ucrt64/mingw-w64-x86_64-SDL2_image\
 ucrt64/mingw-w64-x86_64-SDL2_mixer\
 ucrt64/mingw-w64-x86_64-SDL2_ttf

# additional development tools
pacman -S mingw-w64-x86_64-clang\
 mingw-w64-x86_64-clang-tools-extra\
 ucrt64/mingw-w64-x86_64-include-what-you-use\
 msys/python

# this linker is much faster on Windows
pacman -S ucrt64/mingw-w64-x86_64-lld
```

### Clangd Setup

Use https://github.com/nickdiego/compiledb to generate the json file clangd needs to debug.

Install with

```
pip install compiledb
# OR
python3 -m pip install compiledb
```

Then run

```
cd scripts
./compile-commands.sh
```

This should allow vscode /w clangd or other programs to ingest the file for usage in development.

### Localization

Localization files are generated with a python script. These should be dropped into the src/assets folder automatically and wont automatically override previous strings.

```
python3 scripts/scan_locstr.py --languages la en
```

The localization doesn't work for the MiyooA30 because the json library that the sdl 2 wrapper uses is not supported by the version of gcc on the distro.

### Emscripten

Install Emscripten the normal way using git.
```
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

If there are cert errors, then the SSL_CERT_FILE bash variable needs to be set.  This can be discovered with python.

```
pip install certifi
python3

# inside python cli
import certifi
print(certifi.where())
quit()

# set the path that was printed above (MSYS2 requires cygwin path parser) for example:
export SSL_CERT_FILE=$(cygpath -u "C:/progs/msys2/ucrt64/lib/python3.12/site-packages/certifi/cacert.pem")
```

### MiyooA30

This repo contains a toolchain for building to the MiyooA30 handheld. This was based off the article that can be found here: https://www.hydrogen18.com/blog/compiling-for-the-miyoo-a30-sdl2.html which has an associated repo here: https://codeberg.org/hydrogen18/miyooa30. That repo is included in this repo as an example under the miyooa30-toolchain folder.

This is all assuming you're running Spruce on your MiyooA30. It should probably work without that, but it hasn't been tested under any other launchers.

Setup the toolchain by building the docker container. Make sure you have docker installed and can run docker commands.

```
# Build the toolchain docker container.
./setup-miyooa30-toolchain.sh

# Run the build command.    This should create a dist folder with executable and assets.
# for msys2 you can run this helper script
cd scripts
./miyooa30-build.sh

# for mac/linux run this command (this wont create a tarfile for you, you'll have to do that yourself)
docker run --rm -it --mount type=bind,source="$(pwd)/src",target=/workspace/src miyooa30-toolchain bash -c "cd /workspace/src && source ~/config.sh && make miyooa30"

# If you want to debug the container, you can get a shell with this (linux/mac command, use script for MSYS2)
docker run --rm -it --mount type=bind,source="$(pwd)/src",target=/workspace/src --entrypoint bash miyooa30-toolchain
```

To get this running on a MiyooA30, copy the contents of the src/dist folder onto the sdcard. This would be the binary and assets folder into a folder at the top level. Then copy the file `scripts/<GameName>.sh` into `Roms/PORTS`.

```
tar -xzvf <GameName>.tar.gz
```

NOTE: Don't put the game's folder in the Roms/PORTS directory, this will cause it to not work, even if you edit the sh file to point to it. For whatever reason, homebrew doesn't work from there (at least when using Spruce). This caused me much frustration.

You can add a .png image of the game with the same name as the sh file in the Roms/PORTS/ image folder if you want an image for the game to appear when you hover it.
