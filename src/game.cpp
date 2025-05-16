#include "game.h"
#include <iostream>

using namespace std;

Game::Game() : 
    currentState(MENU), 
    selectedMenuItem(0), 
    selectedLevelIndex(0),
    firstVisibleLevel(0),
    currentLevel(nullptr) {
    
    // Ініціалізуємо пункти меню
    menuItems[0] = "Select Level";
    menuItems[1] = "Create Level (Not Avaliabble)";
    menuItems[2] = "Generate Level (Not Avaliabble)";
    menuItems[3] = "Settings (Not Avaliabble)";
    
    // Встановлюємо шляхи
    fontPath = "C:\\Users\\Maxim\\Desktop\\iasa\\icn bin ein programist\\KURSACH\\pushpush\\assetst\\DroidSans.ttf";
    levelsPath = "C:\\Users\\Maxim\\Desktop\\iasa\\icn bin ein programist\\KURSACH\\pushpush\\assetst\\levels";
}

Game::~Game() {
    cleanup();
}

bool Game::initialize() {
    // Ініціалізуємо SDL і TTF
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL_Init Error: " << SDL_GetError() << endl;
        return false;
    }
    
    if (TTF_Init() < 0) {
        cerr << "TTF_Init Error: " << SDL_GetError() << endl;
        SDL_Quit();
        return false;
    }
    
    // Ініціалізуємо рендерер
    if (!renderer.initialize(fontPath)) {
        cerr << "Failed to initialize renderer" << endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    // Створюємо об'єкт рівня
    currentLevel = new Level(levelsPath);
    if (!currentLevel) {
        cerr << "Failed to create level object" << endl;
        renderer.cleanup();
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    if (!audioManager.initialize()) {
        cerr << "Failed to initialize audio" << endl;
        // Продовжуємо без звуку
    }
    else {
        // Завантажуємо звуки
        audioManager.loadSound("win", "C:\\Users\\Maxim\\Desktop\\iasa\\icn bin ein programist\\KURSACH\\pushpush\\assetst\\sounds\\win.wav");
        audioManager.loadSound("lose", "C:\\Users\\Maxim\\Desktop\\iasa\\icn bin ein programist\\KURSACH\\pushpush\\assetst\\sounds\\lose.wav");
        audioManager.loadSound("jump", "C:\\Users\\Maxim\\Desktop\\iasa\\icn bin ein programist\\KURSACH\\pushpush\\assetst\\sounds\\jump.wav");
    }
    // Завантажуємо список рівнів
    refreshLevelList();
    
    // Якщо немає рівнів, створюємо рівень за замовчуванням
    if (levelFiles.empty()) {
        currentLevel->createDefaultLevel();
    }
    
    // Розраховуємо масштаб відображення рівня
    renderer.calculateScaling(currentLevel->getWidth(), currentLevel->getHeight());
    
    return true;
}

void Game::cleanup() {
    if (currentLevel) {
        delete currentLevel;
        currentLevel = nullptr;
    }
    
    renderer.cleanup();
    TTF_Quit();
    SDL_Quit();
}

void Game::refreshLevelList() {
    levelFiles = currentLevel->getLevelFileList();
    selectedLevelIndex = 0;
    firstVisibleLevel = 0;
}

void Game::loadSelectedLevel() {
    if (!levelFiles.empty()) {
        if (currentLevel->loadLevelFromFile(levelFiles[selectedLevelIndex])) {
            currentState = GAME_PLAYING;
            renderer.calculateScaling(currentLevel->getWidth(), currentLevel->getHeight());
        }
    }
}

void Game::handleMainMenuInput(SDL_Event& e) {
    if (e.type == SDL_EVENT_KEY_DOWN) {
        switch (e.key.key) {
        case SDLK_UP:
            selectedMenuItem = (selectedMenuItem - 1 + MENU_ITEMS) % MENU_ITEMS;
            break;
        case SDLK_DOWN:
            selectedMenuItem = (selectedMenuItem + 1) % MENU_ITEMS;
            break;
        case SDLK_RETURN: case SDLK_SPACE:
            // Обробка вибору пункту меню
            if (selectedMenuItem == 0) { // Вибрати рівень
                currentState = LEVEL_SELECT;
                refreshLevelList(); // Оновлюємо список рівнів
            }
            break;
        case SDLK_ESCAPE:
            SDL_Event quitEvent;
            quitEvent.type = SDL_EVENT_QUIT;
            SDL_PushEvent(&quitEvent);
            break;
        }
    }
}

void Game::handleLevelSelectInput(SDL_Event& e) {
    if (e.type == SDL_EVENT_KEY_DOWN) {
        switch (e.key.key) {
        case SDLK_UP:
            if (!levelFiles.empty()) {
                selectedLevelIndex = (selectedLevelIndex - 1 + levelFiles.size()) % levelFiles.size();
            }
            break;
        case SDLK_DOWN:
            if (!levelFiles.empty()) {
                selectedLevelIndex = (selectedLevelIndex + 1) % levelFiles.size();
            }
            break;
        case SDLK_RETURN: case SDLK_SPACE:
            // Завантаження вибраного рівня
            loadSelectedLevel();
            break;
        case SDLK_ESCAPE:
            currentState = MENU; // Повернення до головного меню
            break;
        }
    }
}

void Game::handleGameInput(SDL_Event& e) {
    if (e.type == SDL_EVENT_KEY_DOWN) {
        if (currentLevel->isLevelFinished() || currentLevel->isLevelFailed()) {
            // Якщо рівень вже завершено, дозволяємо тільки перезапуск або вихід
            switch (e.key.key) {
            case SDLK_ESCAPE:
                currentState = MENU; // Повернення до головного меню
                break;
            case SDLK_R: // Перезапуск рівня
                if (!levelFiles.empty()) {
                    currentLevel->loadLevelFromFile(levelFiles[selectedLevelIndex]);
                }
                else {
                    currentLevel->createDefaultLevel();
                }
                currentLevel->reset(); // Скидаємо стан рівня
                break;
            case SDLK_F: // Перемикання повноекранного режиму
                renderer.toggleFullscreen();
                break;
            }
        }
        else {
            // Звичайна обробка для гри
            switch (e.key.key) {
            case SDLK_W: case SDLK_UP:
                audioManager.playSound("jump");
                currentLevel->movePlayer('w');
                break;
            case SDLK_S: case SDLK_DOWN:
                audioManager.playSound("jump");
                currentLevel->movePlayer('s');
                break;
            case SDLK_A: case SDLK_LEFT:
                audioManager.playSound("jump");
                currentLevel->movePlayer('a');
                break;
            case SDLK_D: case SDLK_RIGHT:
                audioManager.playSound("jump");
                currentLevel->movePlayer('d');
                break;
            case SDLK_ESCAPE:
                currentState = MENU; // Повернення до головного меню
                break;
            case SDLK_R: // Перезапуск рівня
                if (!levelFiles.empty()) {
                    currentLevel->loadLevelFromFile(levelFiles[selectedLevelIndex]);
                }
                else {
                    currentLevel->createDefaultLevel();
                }
                currentLevel->reset(); // Скидаємо стан рівня
                break;
            case SDLK_F: // Перемикання повноекранного режиму
                renderer.toggleFullscreen();
                break;
            }

            // Перевіряємо умови перемоги чи поразки після руху
            if (currentLevel->isLevelFinished()) {
                audioManager.playSound("win");
            }
            else if (currentLevel->isLevelFailed()) {
                audioManager.playSound("lose");
            }
        }
    }
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            cleanup();
            exit(0);
        }
        else if (e.type == SDL_EVENT_WINDOW_RESIZED) {
            renderer.calculateScaling(currentLevel->getWidth(), currentLevel->getHeight());
        }
        else {
            // Обробка введення відповідно до поточного стану
            switch (currentState) {
            case MENU:
                handleMainMenuInput(e);
                break;
            case LEVEL_SELECT:
                handleLevelSelectInput(e);
                break;
            case GAME_PLAYING:
                handleGameInput(e);
                break;
            }
        }
    }
}

void Game::run() {
    bool quit = false;

    while (!quit) {
        // Отримуємо поточний час для анімацій
        Uint32 currentTime = SDL_GetTicks();

        // Оновлюємо анімації
        currentLevel->updateAnimations(currentTime);

        // Обробка подій
        handleEvents();

        // Відмальовуємо відповідно до поточного стану
        switch (currentState) {
        case MENU:
            renderer.drawMainMenu(selectedMenuItem, menuItems);
            break;
        case LEVEL_SELECT:
            renderer.drawLevelSelect(levelFiles, selectedLevelIndex, firstVisibleLevel);
            break;
        case GAME_PLAYING:
            renderer.drawLevel(*currentLevel);
            break;
        }

        SDL_Delay(16); // Приблизно 60 FPS
    }
}