#pragma once

#include "audio.h"
#include "constants.h"
#include "level.h"
#include "renderer.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>

class Game {
private:
    GameState currentState;
    int selectedMenuItem;
    int selectedLevelIndex;
    int firstVisibleLevel;
    std::vector<std::string> levelFiles;
    std::string menuItems[MENU_ITEMS];
    
    std::string fontPath;
    std::string levelsPath;
    
    // Основні компоненти
    Renderer renderer;
    Level* currentLevel;
    AudioManager audioManager;

public:
    Game();
    ~Game();
    
    bool initialize();
    void cleanup();
    void run();
    
private:
    void refreshLevelList();
    void handleEvents();
    void handleMainMenuInput(SDL_Event& e);
    void handleLevelSelectInput(SDL_Event& e);
    void handleGameInput(SDL_Event& e);
    void loadSelectedLevel();
};
