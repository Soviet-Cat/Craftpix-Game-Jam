#include "glb.hpp"

const int glb::WINDOW_WIDTH = 1280;
const int glb::WINDOW_HEIGHT = 720;
const char* glb::WINDOW_TITLE = "CyberSwitch";

SDL_Event* glb::event = nullptr;
SDL_Window** glb::window = nullptr;
SDL_GLContext* glb::context = nullptr;