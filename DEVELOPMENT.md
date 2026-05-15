# Development

Development Requirements

- GCC/Clang
- SDL2
- SDL2_image
- SDL2_ttf
- SDL2_mixer
- SDL2_gfx

For IDE
- clangd
- include-what-you-use
- python3
- compiledb

For Building WASM Executable

- Emscripten

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

If you don't have a windows gcc env, here's a way to set it up with some powershell scripts:

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
 ucrt64/mingw-w64-x86_64-SDL2_ttf\
 ucrt64/mingw-w64-x86_64-SDL2_gfx

# additional development tools
pacman -S mingw-w64-x86_64-clang\
 mingw-w64-x86_64-clang-tools-extra\
 ucrt64/mingw-w64-x86_64-include-what-you-use\
 msys/python

# this linker is much faster on Windows
pacman -S ucrt64/mingw-w64-ucrt-x86_64-lld
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
