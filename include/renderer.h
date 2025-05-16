#pragma once

#include "constants.h"
#include "level.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>

class Renderer {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* titleFont;
    TTF_Font* menuFont;
    TTF_Font* smallFont;
    TTF_Font* gameFont;

    bool isFullscreen;

    // Змінні для масштабування
    float cellSize;
    float offsetX;
    float offsetY;

    // Кольори
    SDL_Color wallColor;
    SDL_Color emptyColor;
    SDL_Color trapColor;
    SDL_Color finishColor;
    SDL_Color startColor;
    SDL_Color playerColor;
    SDL_Color menuBgColor;
    SDL_Color menuItemColor;
    SDL_Color selectedTextColor;

    // Функції для рендерингу тексту
    SDL_Texture* createTextTexture(const std::string& text, SDL_Color color, TTF_Font* font);
    void renderText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* font);

public:
    Renderer();
    ~Renderer();

    bool initialize(const std::string& fontPath);
    void cleanup();

    // Функції для відображення
    void calculateScaling(int levelWidth, int levelHeight);
    void toggleFullscreen();

    // Відображення різних станів гри
    void drawMainMenu(int selectedMenuItem, const std::string menuItems[]);
    void drawLevelSelect(const std::vector<std::string>& levelFiles, int selectedLevelIndex, int firstVisibleLevel);
    void drawLevel(const Level& level);

    // Нові функції для геймплейних механік
    void drawLevelWin(const Level& level);
    void drawLevelLose(const Level& level);
    void drawTrail(const std::vector<Level::TrailPoint>& trail, Uint32 currentTime);

    // Отримуємо вікно і рендерер
    SDL_Window* getWindow() const { return window; }
    SDL_Renderer* getRenderer() const { return renderer; }
};