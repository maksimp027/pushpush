#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;
using namespace std;

// Константи для типів клітинок
const char WALL = 'x', EMPTY = '_', START = 's', FINISH = 'f', TRAP = 'd', PLAYER = 'P';

// Стани гри
enum GameState {
    MENU,
    LEVEL_SELECT,
    GAME_PLAYING
};

// Поточний стан гри
GameState currentState = MENU;

// Динамічні дані рівня
int WIDTH = 0, HEIGHT = 0;
std::vector<std::vector<char>> level;
int playerX = 0, playerY = 0;

// Дані для меню
int selectedMenuItem = 0;
const int MENU_ITEMS = 4;
std::string menuItems[MENU_ITEMS] = {
    "Select Level",
    "Create Level (Not Avaliabble)",
    "Generate Level (Not Avaliabble)",
    "Settings (Not Avaliabble)"
};

// Дані для провідника рівнів
std::vector<std::string> levelFiles;
int selectedLevelIndex = 0;
int firstVisibleLevel = 0;
const int MAX_VISIBLE_LEVELS = 10;
std::string levelsPath = "C:\\Users\\Maxim\\Desktop\\iasa\\icn bin ein programist\\KURSACH\\pushpush\\assetst\\levels";

// Шлях до шрифту
std::string fontPath = "C:\\Users\\Maxim\\Desktop\\iasa\\icn bin ein programist\\KURSACH\\pushpush\\assetst\\DroidSans.ttf";
// Кольори для різних елементів гри
SDL_Color wallColor = { 150, 150, 150, 255 };       // Колір стін - сірий
SDL_Color emptyColor = { 244, 244, 240, 255 };      // Колір порожніх клітин - білий
SDL_Color trapColor = { 196, 36, 44, 255 };         // Колір пасток - червоний
SDL_Color finishColor = { 42, 166, 220, 255 };      // Колір фінішу - синій
SDL_Color startColor = { 252, 252, 177, 255 };      // Колір старту - світло-жовтий
SDL_Color playerColor = { 252, 236, 36, 255 };      // Колір гравця - яскраво-жовтий
SDL_Color menuBgColor = { 244, 244, 240, 255 };     // Колір фону меню - такий як порожні клітини
SDL_Color menuItemColor = { 255, 255, 255, 255 };   // Колір пунктів меню - ідеально білий
SDL_Color selectedTextColor = { 0, 0, 0, 255 };     // Колір тексту вибраного пункту - ідеально чорний

SDL_Window* window;
SDL_Renderer* renderer;
TTF_Font* titleFont = nullptr;   // Шрифт для заголовка (60px)
TTF_Font* menuFont = nullptr;    // Шрифт для пунктів меню (24px)
TTF_Font* smallFont = nullptr;   // Шрифт для пояснень (16px)
TTF_Font* gameFont = nullptr;    // Основний шрифт для гри (24px)

// Розміри вікна
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 900;

// Фіксована висота ігрового поля
const int GAME_FIELD_HEIGHT = 600;

bool isFullscreen = false;

// Змінні для масштабування
float cellSize;
float offsetX;
float offsetY;

// Функція для створення текстури з тексту
SDL_Texture* createTextTexture(const std::string& text, SDL_Color color, TTF_Font* font) {
    if (!font) {
        cerr << "Font not loaded!" << endl;
        return nullptr;
    }

    // Створюємо поверхню з тексту
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

// Функція для відображення тексту
void renderText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* font) {
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

// Функція для видалення розширення файлу
std::string removeFileExtension(const std::string& filename) {
    size_t lastDotPos = filename.find_last_of(".");
    if (lastDotPos == std::string::npos) {
        return filename;
    }
    return filename.substr(0, lastDotPos);
}

// Завантаження списку доступних рівнів
void refreshLevelList() {
    levelFiles.clear();
    selectedLevelIndex = 0;
    firstVisibleLevel = 0;

    try {
        // Перевіряємо чи існує директорія
        if (!fs::exists(levelsPath)) {
            fs::create_directories(levelsPath);
            cout << "Created levels directory" << endl;
            return;
        }

        // Отримуємо список .bin файлів
        for (const auto& entry : fs::directory_iterator(levelsPath)) {
            if (entry.path().extension() == ".bin") {
                levelFiles.push_back(entry.path().filename().string());
            }
        }

        // Сортуємо список
        std::sort(levelFiles.begin(), levelFiles.end());
    }
    catch (const fs::filesystem_error& e) {
        cerr << "Error accessing directory: " << e.what() << endl;
    }
}

// Функція для завантаження рівня з бінарного файлу
bool loadLevelFromFile(const std::string& filename) {
    std::string filePath = levelsPath + "\\" + filename;

    std::ifstream inFile(filePath, std::ios::binary | std::ios::in);

    if (!inFile) {
        cerr << "Failed to open level file: " << filePath << endl;
        return false;
    }

    // Зчитуємо розміри рівня
    inFile.read(reinterpret_cast<char*>(&WIDTH), sizeof(WIDTH));
    inFile.read(reinterpret_cast<char*>(&HEIGHT), sizeof(HEIGHT));

    // Перевіряємо, чи розміри знаходяться в розумних межах
    if (WIDTH <= 0 || WIDTH > 100 || HEIGHT <= 0 || HEIGHT > 100) {
        cerr << "Invalid level dimensions: " << WIDTH << "x" << HEIGHT << endl;
        return false;
    }

    // Очищаємо існуючий рівень і створюємо новий
    level.clear();
    level.resize(HEIGHT, std::vector<char>(WIDTH));

    // Зчитуємо дані рівня
    for (int i = 0; i < HEIGHT; i++) {
        char rowBuffer[100]; // Достатній буфер
        inFile.read(rowBuffer, WIDTH);
        for (int j = 0; j < WIDTH; j++) {
            level[i][j] = rowBuffer[j];
        }
    }

    // Зчитуємо позицію гравця
    inFile.read(reinterpret_cast<char*>(&playerX), sizeof(playerX));
    inFile.read(reinterpret_cast<char*>(&playerY), sizeof(playerY));

    inFile.close();

    cout << "Loaded level: " << WIDTH << "x" << HEIGHT << ", player at (" << playerX << ", " << playerY << ")" << endl;
    return true;
}

// Функція для створення рівня за замовчуванням, якщо не вдалося завантажити з файлу
void createDefaultLevel() {
    WIDTH = 8;
    HEIGHT = 9;

    level.resize(HEIGHT, std::vector<char>(WIDTH));

    // Заповнюємо рівень за замовчуванням
    const char defaultLevel[9][8] = {
        {'x','x','x','x','x','x','x','x'},
        {'x','f','_','x','x','x','_','x'},
        {'x','_','_','x','x','x','_','x'},
        {'x','_','_','_','_','_','_','x'},
        {'x','x','x','x','x','_','x','x'},
        {'x','d','_','_','x','_','_','x'},
        {'x','_','_','_','x','_','_','x'},
        {'x','s','_','_','_','_','_','x'},
        {'x','x','x','x','x','x','x','x'}
    };

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            level[i][j] = defaultLevel[i][j];
        }
    }

    playerX = 1;
    playerY = 7;

    cout << "Using default level" << endl;
}

void toggleFullscreen() {
    isFullscreen = !isFullscreen;
    if (isFullscreen) {
        SDL_SetWindowFullscreen(window, true);
    }
    else {
        SDL_SetWindowFullscreen(window, false);
    }
}

void calculateScaling() {
    if (HEIGHT <= 0) return; // Запобігаємо діленню на нуль

    // Розмір клітинки залежить тільки від висоти ігрового поля і кількості клітинок
    cellSize = (float)GAME_FIELD_HEIGHT / HEIGHT;

    // Загальна ширина поля в пікселях
    float totalWidth = cellSize * WIDTH;

    // Отримуємо поточні розміри вікна
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // Обчислюємо відступи для центрування карти
    offsetX = (windowWidth - totalWidth) / 2;
    offsetY = (windowHeight - (float)GAME_FIELD_HEIGHT) / 2;
}

void drawMainMenu() {
    // Використовуємо світло-сірий колір для фону меню
    SDL_SetRenderDrawColor(renderer, 244, 244, 240, 255); // Світло-сірий фон
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
            textColor = { 0, 0, 0, 255 }; // Чорний текст для вибраного пункту
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

void drawLevelSelect() {
    // Використовуємо світло-сірий колір для фону меню
    SDL_SetRenderDrawColor(renderer, 244, 244, 240, 255); // Світло-сірий фон
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

    // Обмежуємо firstVisibleLevel, щоб він не виходив за межі
    if (selectedLevelIndex < firstVisibleLevel) {
        firstVisibleLevel = selectedLevelIndex;
    }
    else if (selectedLevelIndex >= firstVisibleLevel + MAX_VISIBLE_LEVELS) {
        firstVisibleLevel = selectedLevelIndex - MAX_VISIBLE_LEVELS + 1;
    }

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
            std::string levelName = removeFileExtension(levelFiles[levelIndex]);

            // Визначаємо колір тексту
            SDL_Color textColor;
            if (levelIndex == selectedLevelIndex) {
                textColor = { 0, 0, 0, 255 }; // Чорний текст для вибраного рівня
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

void drawLevel() {
    // Змінюємо колір фону на той самий, що й порожні клітинки
    SDL_SetRenderDrawColor(renderer, emptyColor.r, emptyColor.g, emptyColor.b, emptyColor.a);
    SDL_RenderClear(renderer);

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            SDL_FRect cellRect;
            cellRect.x = offsetX + j * cellSize;
            cellRect.y = offsetY + i * cellSize;
            cellRect.w = cellSize;
            cellRect.h = cellSize;

            if (i == playerY && j == playerX) {
                // Гравець пріоритетніший від клітинки
                SDL_SetRenderDrawColor(renderer, playerColor.r, playerColor.g, playerColor.b, playerColor.a);
            }
            else if (level[i][j] == WALL) {
                SDL_SetRenderDrawColor(renderer, wallColor.r, wallColor.g, wallColor.b, wallColor.a);
            }
            else if (level[i][j] == EMPTY) {
                SDL_SetRenderDrawColor(renderer, emptyColor.r, emptyColor.g, emptyColor.b, emptyColor.a);
            }
            else if (level[i][j] == TRAP) {
                SDL_SetRenderDrawColor(renderer, trapColor.r, trapColor.g, trapColor.b, trapColor.a);
            }
            else if (level[i][j] == FINISH) {
                SDL_SetRenderDrawColor(renderer, finishColor.r, finishColor.g, finishColor.b, finishColor.a);
            }
            else if (level[i][j] == START) {
                SDL_SetRenderDrawColor(renderer, startColor.r, startColor.g, startColor.b, startColor.a);
            }

            SDL_RenderFillRect(renderer, &cellRect);
        }
    }

    // Інформація про рівень (без розширення .bin)
    std::string levelInfo = "Level: ";
    if (levelFiles.empty()) {
        levelInfo += "Default";
    }
    else {
        levelInfo += removeFileExtension(levelFiles[selectedLevelIndex]);
    }

    renderText(levelInfo, 20, 20, wallColor, menuFont);

    // Інструкції
    renderText("R - Restart   ESC - Menu   F - Fullscreen", 20, WINDOW_HEIGHT - 40, wallColor, smallFont);

    SDL_RenderPresent(renderer);
}

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {  // 1 означає успішну ініціалізацію
        cout << "SDL_Init Error: " << SDL_GetError() << endl;
        exit(1);
    }

    // Ініціалізація TTF
    if (TTF_Init() < 0) {
        cout << "TTF_Init Error: " << SDL_GetError() << endl;
        SDL_Quit();
        exit(1);
    }

    // Створення вікна з правильними параметрами для SDL3
    window = SDL_CreateWindow("Push-Push Game", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        cout << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

    // Завантаження шрифтів різних розмірів
    titleFont = TTF_OpenFont(fontPath.c_str(), 60);  // Великий шрифт для заголовка (60px)
    if (!titleFont) {
        cout << "Failed to load title font: " << SDL_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

    menuFont = TTF_OpenFont(fontPath.c_str(), 24);  // Середній шрифт для меню (24px)
    if (!menuFont) {
        cout << "Failed to load menu font: " << SDL_GetError() << endl;
        TTF_CloseFont(titleFont);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

    smallFont = TTF_OpenFont(fontPath.c_str(), 16);  // Маленький шрифт для пояснень (16px)
    if (!smallFont) {
        cout << "Failed to load small font: " << SDL_GetError() << endl;
        TTF_CloseFont(titleFont);
        TTF_CloseFont(menuFont);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

    gameFont = menuFont; // Використовуємо меню шрифт як основний для гри

    // Завантажуємо список рівнів
    refreshLevelList();
}

// Функція перевіряє, чи клітинка - перешкода (стіна)
bool isObstacle(int x, int y) {
    // Перевіряємо чи координати в межах поля і чи це не стіна
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
        return true; // За межами поля вважаємо перешкодою
    }
    return level[y][x] == WALL;
}

void movePlayer(char direction) {
    int newX = playerX, newY = playerY;

    // Використовуємо напрямок руху для визначення інкременту
    int dx = 0, dy = 0;

    switch (direction) {
    case 'w': // Вгору
        dy = -1;
        break;
    case 's': // Вниз
        dy = 1;
        break;
    case 'a': // Вліво
        dx = -1;
        break;
    case 'd': // Вправо
        dx = 1;
        break;
    }

    // Ковзаємо поки не зустрінемо перешкоду
    while (!isObstacle(newX + dx, newY + dy)) {
        newX += dx;
        newY += dy;
    }

    // Оновлюємо позицію тільки якщо вона змінилась
    if (newX != playerX || newY != playerY) {
        playerX = newX;
        playerY = newY;

        if (level[playerY][playerX] == TRAP) {
            // Гравець потрапив у пастку - повертаємося в меню
            currentState = MENU;
        }
        if (level[playerY][playerX] == FINISH) {
            // Гравець досяг фінішу - повертаємося в меню
            currentState = MENU;
        }
    }
}

void handleMainMenuInput(SDL_Event& e) {
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

void handleLevelSelectInput(SDL_Event& e) {
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
            if (!levelFiles.empty()) {
                if (loadLevelFromFile(levelFiles[selectedLevelIndex])) {
                    currentState = GAME_PLAYING;
                    calculateScaling();
                }
            }
            break;
        case SDLK_ESCAPE:
            currentState = MENU; // Повернення до головного меню
            break;
        }
    }
}

void handleGameInput(SDL_Event& e) {
    if (e.type == SDL_EVENT_KEY_DOWN) {
        switch (e.key.key) {
        case SDLK_W: case SDLK_UP:
            movePlayer('w');
            break;
        case SDLK_S: case SDLK_DOWN:
            movePlayer('s');
            break;
        case SDLK_A: case SDLK_LEFT:
            movePlayer('a');
            break;
        case SDLK_D: case SDLK_RIGHT:
            movePlayer('d');
            break;
        case SDLK_ESCAPE:
            currentState = MENU; // Повернення до головного меню
            break;
        case SDLK_R: // Перезапуск рівня
            if (!levelFiles.empty()) {
                loadLevelFromFile(levelFiles[selectedLevelIndex]);
            }
            else {
                createDefaultLevel();
            }
            break;
        case SDLK_F: // Перемикання повноекранного режиму
            toggleFullscreen();
            break;
        }
    }
}

int main() {
    SDL_Event e;
    bool quit = false;

    initSDL();

    // Якщо немає рівнів, створюємо рівень за замовчуванням
    if (levelFiles.empty()) {
        createDefaultLevel();
    }

    calculateScaling(); // Додано розрахунок масштабу при запуску

    while (!quit) {
        // Відмальовуємо відповідно до поточного стану
        switch (currentState) {
        case MENU:
            drawMainMenu();
            break;
        case LEVEL_SELECT:
            drawLevelSelect();
            break;
        case GAME_PLAYING:
            drawLevel();
            break;
        }

        // Обробка подій
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                calculateScaling();
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

        SDL_Delay(16); // Приблизно 60 FPS
    }

    // Звільняємо ресурси
    TTF_CloseFont(titleFont);
    TTF_CloseFont(menuFont);
    TTF_CloseFont(smallFont);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}