#include "game.h"
#include <iostream>

int main() {
    std::cout << "Git Fighter - 救火架构师" << std::endl;
    std::cout << "Starting game..." << std::endl;

    GitGame game;

    if (!game.Initialize()) {
        std::cerr << "Failed to initialize game" << std::endl;
        return 1;
    }

    game.Run();
    game.Shutdown();

    return 0;
}
