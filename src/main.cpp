#include "glb.hpp"

#include "game.hpp"

void loop()
{
    Game::frame();
}

int main()
{
    int result = Game::init();
    if (result != 0)
    {
        std::cout << "Exited with result: " << result << std::endl;
        return result;
    }

    emscripten_set_main_loop(loop, 0, 1);
    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
}