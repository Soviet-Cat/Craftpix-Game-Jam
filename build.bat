@echo off
setlocal enabledelayedexpansion

echo Setting environment...

set EMSDK_QUIET=1
call emsdk_env
echo Set environment finished...

echo Detecting source files...

set SRC_FILES=
for %%f in (src\*.cpp) do set SRC_FILES=!SRC_FILES! %%f
echo !SRC_FILES!

echo Detect source files finished...

echo Building prerequisites...

call embuilder build sdl2 sdl2_ttf sdl2_image

echo Build prerequisites finished...

echo Building...

call em++ %SRC_FILES% ^
    -o cyberswitch.js ^
    --preload-file assets --preload-file shaders ^
    -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS=png -s USE_SDL_MIXER=2 -s USE_SDL_TTF=2 ^
    -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 -s FULL_ES3=1 -lGL ^
    -s ALLOW_MEMORY_GROWTH=1 -fwasm-exceptions

echo Build finished...