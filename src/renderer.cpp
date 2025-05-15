#include "renderer.h"
#include "constants.h"
#include "utils.h"
#include <iostream>

using namespace std;

Renderer::Renderer() : window(nullptr), renderer(nullptr), 
                       titleFont(nullptr), menuFont(nullptr), 
                       smallFont(nullptr), gameFont(nullptr),
                       isFullscreen(false), cellSize(0), offsetX(0), offsetY(0) {
    // Ініціалізація кольорів
    wallColor = { 150, 150, 150, 255 };       // Колір стін - сірий
    emptyColor = { 244, 244, 240, 255 };      // Колір порожніх клітин - білий
    trapColor = { 196, 36, 44, 255 };         // Колір пасток - червоний
    finishColor = { 42, 166, 220, 255 };      // Колір фінішу - синій
    startColor = { 252, 252, 177, 255 };      // Колір старту - світло-жовтий
    playerColor = { 252, 236, 36, 255 };      // Колір гравця - яскраво-жовтий
    menuBgColor = { 244, 244, 240, 255 };     // Колір фону меню - такий як порожні клітини
    menuItemColor = { 255, 255, 255, 255 };   // Колір пунктів меню - ідеально білий
    selectedTextColor = { 0, 0, 0, 255 };     // Колір тексту вибраного пункту - ідеально чорний
}

Renderer::~Renderer() {
    cleanup();
}

bool Renderer::initialize(const std::string& fontPath) {
    // Створення вікна з правильними параметрами для SDL3
    window = SDL_CreateWindow("Push-Push Game", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        return false;
    }

    // Завантаження шрифтів різних розмірів
    titleFont = TTF_OpenFont(fontPath.c_str(), 60);  // Великий шрифт для заголовка (60px)
    if (!titleFont) {
        cerr << "Failed to load title font: " << SDL_GetError() << endl;
        cerr << "Font path: " << fontPath << endl;
        return false;
    }

    menuFont = TTF_OpenFont(fontPath.c_str(), 24);  // Середній шрифт для меню (24px)
    if (!menuFont) {
        cerr << "Failed to load menu font: " << SDL_GetError() << endl;
        return false;
    }

    smallFont = TTF_OpenFont(fontPath.c_str(), 16);  // Маленький шрифт для пояснень (16px)
    if (!smallFont) {
        cerr << "Failed to load small font: " << SDL_GetError() << endl;
        return false;
    }

    gameFont = menuFont; // Використовуємо меню шрифт як основний для гри
    return true;
}

void Renderer::cleanup() {
    if (titleFont) TTF_CloseFont(titleFont);
    if (menuFont) TTF_CloseFont(menuFont);
    if (smallFont) TTF_CloseFont(smallFont);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    
    titleFont = nullptr;
    menuFont = nullptr;
    smallFont = nullptr;
    renderer = nullptr;
    window = nullptr;
}

SDL_Texture* Renderer::createTextTexture(const std::string& text, SDL_Color color, TTF_Font* font) {
    if (!font) {
        cerr << "Font not loaded!" << endl;
        return nullptr;
    }

    // Створюємо поверхню з тексту (виправлений виклик для SDL3)
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), text.length(), color);
    if (!textSurface) {
        cerr << "Failed to render text: " << SDL_GetError() << endl;
        return nullptr;
    }

    // Створюємо текстуру з поверхні
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_DestroySurface(textSurface);

    if (!texture) {
        cerr << "Failed to create texture from text: " << SDL_GetError() << endl;
    }

    return texture;
}

void Renderer::renderText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* font) {
    SDL_Texture* textTexture = createTextTexture(text, color, font);
    if (!textTexture) return;

    // Отримуємо розмір текстури
    float width, height;
    SDL_GetTextureSize(textTexture, &width, &height);

    // Встановлюємо прямокутник для відображення
    SDL_FRect destRect = {
        (float)x,
        (float)y,
        (float)width,
        (float)height
    };

    // Відображаємо текст
    SDL_RenderTexture(renderer, textTexture, NULL, &destRect);

    // Звільняємо ресурси
    SDL_DestroyTexture(textTexture);
}

void Renderer::calculateScaling(int levelWidth, int levelHeight) {
    if (levelHeight <= 0) return; // Запобігаємо діленню на нуль

    // Розмір клітинки залежить тільки від висоти ігрового поля і кількості клітинок
    cellSize = (float)GAME_FIELD_HEIGHT / levelHeight;

    // Загальна ширина поля в пікселях
    float totalWidth = cellSize * levelWidth;

    // Отримуємо поточні розміри вікна
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // Обчислюємо відступи для центрування карти
    offsetX = (windowWidth - totalWidth) / 2;
    offsetY = (windowHeight - (float)GAME_FIELD_HEIGHT) / 2;
}

void Renderer::toggleFullscreen() {
    isFullscreen = !isFullscreen;
    SDL_SetWindowFullscreen(window, isFullscreen);
}

void Renderer::drawMainMenu(int selectedMenuItem, const std::string menuItems[]) {
    // Використовуємо світло-сірий колір для фону меню
    SDL_SetRenderDrawColor(renderer, menuBgColor.r, menuBgColor.g, menuBgColor.b, menuBgColor.a);
    SDL_RenderClear(renderer);

    // Отримуємо розміри вікна
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // Центрування заголовка (позиція по горизонталі буде обчислена в renderText)
    int titleY = 100; // Відстань від верху: 100 пікселів

    // Відтворюємо заголовок (центрований)
    SDL_Texture* titleTexture = createTextTexture("Push-Push", { 0, 0, 0, 255 }, titleFont);
    if (titleTexture) {
        float titleWidth, titleHeight;
        SDL_GetTextureSize(titleTexture, &titleWidth, &titleHeight);
        SDL_FRect titleRect = {
            (windowWidth - titleWidth) / 2, // Центрування по горизонталі
            (float)titleY,
            titleWidth,
            titleHeight
        };
        SDL_RenderTexture(renderer, titleTexture, NULL, &titleRect);
        SDL_DestroyTexture(titleTexture);
    }

    // Початкова позиція пунктів меню
    int menuStartY = titleY + 60 + 60; // Заголовок + висота тексту заголовка + відступ
    int menuItemHeight = 60; // Висота кнопки
    int menuItemPadding = 20; // Відстань між кнопками
    int menuWidth = 500; // Ширина кнопок

    // Пункти меню
    for (int i = 0; i < MENU_ITEMS; i++) {
        // Розраховуємо Y-позицію цього пункту меню
        int itemY = menuStartY + i * (menuItemHeight + menuItemPadding);

        // Обчислюємо позицію для центрування по горизонталі
        int itemX = (windowWidth - menuWidth) / 2;

        // Створюємо прямокутник для кнопки
        SDL_FRect itemRect = {
            (float)itemX,
            (float)itemY,
            (float)menuWidth,
            (float)menuItemHeight
        };

        // Малюємо білий фон для всіх кнопок
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &itemRect);

        // Визначаємо колір тексту залежно від того, вибраний пункт чи ні
        SDL_Color textColor;

        if (i == selectedMenuItem) {
            textColor = selectedTextColor; // Чорний текст для вибраного пункту
        }
        else {
            textColor = { 150, 150, 150, 255 }; // Сірий текст для невибраних пунктів
        }

        // Створюємо текстуру для тексту пункту меню
        SDL_Texture* itemTexture = createTextTexture(menuItems[i], textColor, menuFont);
        if (itemTexture) {
            float textWidth, textHeight;
            SDL_GetTextureSize(itemTexture, &textWidth, &textHeight);

            // Центруємо текст в кнопці
            SDL_FRect textRect = {
                (float)(itemX + (menuWidth - textWidth) / 2),
                (float)(itemY + (menuItemHeight - textHeight) / 2),
                textWidth,
                textHeight
            };

            SDL_RenderTexture(renderer, itemTexture, NULL, &textRect);
            SDL_DestroyTexture(itemTexture);
        }
    }

    // Позиція для нижнього напису
    int creditsY = menuStartY + MENU_ITEMS * (menuItemHeight + menuItemPadding) + 60; // Додатковий відступ 60 пікселів

    // Відтворюємо пояснювальний напис - тільки "Term project"
    std::string creditsText = "Term project";

    SDL_Texture* creditsTexture = createTextTexture(creditsText, { 150, 150, 150, 255 }, smallFont);
    if (creditsTexture) {
        float textWidth, textHeight;
        SDL_GetTextureSize(creditsTexture, &textWidth, &textHeight);

        SDL_FRect textRect = {
            (windowWidth - textWidth) / 2,
            (float)creditsY,
            textWidth,
            textHeight
        };

        SDL_RenderTexture(renderer, creditsTexture, NULL, &textRect);
        SDL_DestroyTexture(creditsTexture);
    }

    SDL_RenderPresent(renderer);
}

void Renderer::drawLevelSelect(const std::vector<std::string>& levelFiles, int selectedLevelIndex, int firstVisibleLevel) {
    // Використовуємо світло-сірий колір для фону меню
    SDL_SetRenderDrawColor(renderer, menuBgColor.r, menuBgColor.g, menuBgColor.b, menuBgColor.a);
    SDL_RenderClear(renderer);

    // Отримуємо розміри вікна
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // Центрування заголовка
    int titleY = 100; // Така ж відстань як і в головному меню

    // Відтворюємо заголовок (центрований)
    SDL_Texture* titleTexture = createTextTexture("Select Level", { 0, 0, 0, 255 }, titleFont);
    if (titleTexture) {
        float titleWidth, titleHeight;
        SDL_GetTextureSize(titleTexture, &titleWidth, &titleHeight);
        SDL_FRect titleRect = {
            (windowWidth - titleWidth) / 2, // Центрування по горизонталі
            (float)titleY,
            titleWidth,
            titleHeight
        };
        SDL_RenderTexture(renderer, titleTexture, NULL, &titleRect);
        SDL_DestroyTexture(titleTexture);
    }

    // Початкова позиція списку рівнів
    int levelStartY = titleY + 60 + 60; // Заголовок + висота тексту заголовка + відступ
    int levelItemHeight = 60; // Така ж висота як для кнопок меню
    int levelItemPadding = 20; // Той же відступ
    int levelWidth = 500; // Та ж ширина

    // Якщо немає рівнів
    if (levelFiles.empty()) {
        // Обчислюємо позицію для центрування
        int noLevelX = (windowWidth - levelWidth) / 2;
        int noLevelY = levelStartY;

        SDL_FRect noLevelsRect = {
            (float)noLevelX,
            (float)noLevelY,
            (float)levelWidth,
            (float)levelItemHeight
        };

        // Малюємо білу кнопку
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &noLevelsRect);

        // Створюємо текст "No levels found"
        SDL_Texture* noLevelsTexture = createTextTexture("No levels found", { 150, 150, 150, 255 }, menuFont);
        if (noLevelsTexture) {
            float textWidth, textHeight;
            SDL_GetTextureSize(noLevelsTexture, &textWidth, &textHeight);

            // Центруємо текст в кнопці
            SDL_FRect textRect = {
                (float)(noLevelX + (levelWidth - textWidth) / 2),
                (float)(noLevelY + (levelItemHeight - textHeight) / 2),
                textWidth,
                textHeight
            };

            SDL_RenderTexture(renderer, noLevelsTexture, NULL, &textRect);
            SDL_DestroyTexture(noLevelsTexture);
        }
    }
    // Відображаємо список рівнів
    else {
        int visibleCount = min(MAX_VISIBLE_LEVELS, (int)levelFiles.size() - firstVisibleLevel);

        for (int i = 0; i < visibleCount; i++) {
            int levelIndex = firstVisibleLevel + i;

            // Обчислюємо позицію для рівня
            int itemY = levelStartY + i * (levelItemHeight + levelItemPadding);
            int itemX = (windowWidth - levelWidth) / 2; // Центрування по горизонталі

            SDL_FRect itemRect = {
                (float)itemX,
                (float)itemY,
                (float)levelWidth,
                (float)levelItemHeight
            };

            // Малюємо білий фон для всіх кнопок
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &itemRect);

            // Прибираємо розширення .bin з назви рівня
            std::string levelName = utils::removeFileExtension(levelFiles[levelIndex]);

            // Визначаємо колір тексту
            SDL_Color textColor;
            if (levelIndex == selectedLevelIndex) {
                textColor = selectedTextColor; // Чорний текст для вибраного рівня
            }
            else {
                textColor = { 150, 150, 150, 255 }; // Сірий текст для невибраних
            }

            // Створюємо текстуру для назви рівня
            SDL_Texture* levelTexture = createTextTexture(levelName, textColor, menuFont);
            if (levelTexture) {
                float textWidth, textHeight;
                SDL_GetTextureSize(levelTexture, &textWidth, &textHeight);

                // Центруємо текст в кнопці
                SDL_FRect textRect = {
                    (float)(itemX + (levelWidth - textWidth) / 2),
                    (float)(itemY + (levelItemHeight - textHeight) / 2),
                    textWidth,
                    textHeight
                };

                SDL_RenderTexture(renderer, levelTexture, NULL, &textRect);
                SDL_DestroyTexture(levelTexture);
            }
        }
    }

    // Позиція для інструкцій
    int instructionsY = levelStartY + min(MAX_VISIBLE_LEVELS, (int)levelFiles.size() == 0 ? 1 : (int)levelFiles.size()) * (levelItemHeight + levelItemPadding) + 60;

    // Додаємо інструкції внизу
    std::string instructionsText = "Press ESC to return to menu";

    SDL_Texture* instructTexture = createTextTexture(instructionsText, { 150, 150, 150, 255 }, smallFont);
    if (instructTexture) {
        float textWidth, textHeight;
        SDL_GetTextureSize(instructTexture, &textWidth, &textHeight);

        SDL_FRect textRect = {
            (windowWidth - textWidth) / 2, // Центрування
            (float)instructionsY,
            textWidth,
            textHeight
        };

        SDL_RenderTexture(renderer, instructTexture, NULL, &textRect);
        SDL_DestroyTexture(instructTexture);
    }

    SDL_RenderPresent(renderer);
}

void Renderer::drawLevel(const Level& level) {
    // Змінюємо колір фону на той самий, що й порожні клітинки
    SDL_SetRenderDrawColor(renderer, emptyColor.r, emptyColor.g, emptyColor.b, emptyColor.a);
    SDL_RenderClear(renderer);

    for (int i = 0; i < level.getHeight(); i++) {
        for (int j = 0; j < level.getWidth(); j++) {
            SDL_FRect cellRect;
            cellRect.x = offsetX + j * cellSize;
            cellRect.y = offsetY + i * cellSize;
            cellRect.w = cellSize;
            cellRect.h = cellSize;

            if (i == level.getPlayerY() && j == level.getPlayerX()) {
                // Гравець пріоритетніший від клітинки
                SDL_SetRenderDrawColor(renderer, playerColor.r, playerColor.g, playerColor.b, playerColor.a);
            }
            else {
                char tile = level.getTileAt(j, i);
                switch(tile) {
                    case WALL:
                        SDL_SetRenderDrawColor(renderer, wallColor.r, wallColor.g, wallColor.b, wallColor.a);
                        break;
                    case EMPTY:
                        SDL_SetRenderDrawColor(renderer, emptyColor.r, emptyColor.g, emptyColor.b, emptyColor.a);
                        break;
                    case TRAP:
                        SDL_SetRenderDrawColor(renderer, trapColor.r, trapColor.g, trapColor.b, trapColor.a);
                        break;
                    case FINISH:
                        SDL_SetRenderDrawColor(renderer, finishColor.r, finishColor.g, finishColor.b, finishColor.a);
                        break;
                    case START:
                        SDL_SetRenderDrawColor(renderer, startColor.r, startColor.g, startColor.b, startColor.a);
                        break;
                    default:
                        SDL_SetRenderDrawColor(renderer, emptyColor.r, emptyColor.g, emptyColor.b, emptyColor.a);
                }
            }

            SDL_RenderFillRect(renderer, &cellRect);
        }
    }

    // Інструкції
    renderText("R - Restart   ESC - Menu   F - Fullscreen", 20, WINDOW_HEIGHT - 40, wallColor, smallFont);

    SDL_RenderPresent(renderer);
}