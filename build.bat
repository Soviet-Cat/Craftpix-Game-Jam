@echo off
setlocal enabledelayedexpansion
echo Setting environment...
call emsdk_env
echo Set environment finished...
echo Building...
set SRC_FILES=
for %%f in (src\*.cpp) do set SRC_FILES=!SRC_FILES! %%f
echo Detected source files: !SRC_FILES!
call em++ %SRC_FILES% ^
    -o cyberswitch.html ^
    -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS=png -s USE_SDL_MIXER=2 -s USE_SDL_TTF=2 ^
    -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 -s FULL_ES3=1 -lGL ^
    -s ALLOW_MEMORY_GROWTH=1 -fwasm-exceptions
echo Build finished...