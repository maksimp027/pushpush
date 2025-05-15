#include "game.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        Game game;
        
        if (game.initialize()) {
            game.run();
        }
        else {
            std::cerr << "Game initialization failed." << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
        return 1;
    }
    
    return 0;
}