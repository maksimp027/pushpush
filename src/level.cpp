#include "level.h"
#include "constants.h"
#include "utils.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <cmath>

namespace fs = std::filesystem;
using namespace std;

Level::Level(const std::string& levelsDirectory)
    : width(0), height(0), playerX(0), playerY(0), levelsPath(levelsDirectory),
    isFinished(false), isFailed(false), animationRadius(0), lastAnimationTime(0) {
    utils::ensureDirectoryExists(levelsPath);
}

Level::~Level() {
    // Звільняємо ресурси, якщо потрібно
}

char Level::getTileAt(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return levelData[y][x];
    }
    return WALL; // За межами - вважаємо стіною
}

std::vector<std::string> Level::getLevelFileList() {
    std::vector<std::string> levelFiles;

    try {
        // Перевіряємо чи існує директорія
        if (!fs::exists(levelsPath)) {
            fs::create_directories(levelsPath);
            cout << "Created levels directory: " << levelsPath << endl;
            return levelFiles;
        }

        // Отримуємо список .bin файлів
        for (const auto& entry : fs::directory_iterator(levelsPath)) {
            if (entry.path().extension() == ".bin") {
                levelFiles.push_back(entry.path().filename().string());
                cout << "Found level: " << entry.path().filename().string() << endl;
            }
        }

        // Сортуємо список
        std::sort(levelFiles.begin(), levelFiles.end());
        cout << "Total levels found: " << levelFiles.size() << endl;
    }
    catch (const fs::filesystem_error& e) {
        cerr << "Error accessing directory: " << e.what() << endl;
    }

    return levelFiles;
}

bool Level::loadLevelFromFile(const std::string& filename) {
    std::string filePath = levelsPath + "\\" + filename;

    cout << "Loading level: " << filePath << endl;
    std::ifstream inFile(filePath, std::ios::binary | std::ios::in);

    if (!inFile) {
        cerr << "Failed to open level file: " << filePath << endl;
        return false;
    }

    // Зчитуємо розміри рівня
    inFile.read(reinterpret_cast<char*>(&width), sizeof(width));
    inFile.read(reinterpret_cast<char*>(&height), sizeof(height));

    // Перевіряємо, чи розміри знаходяться в розумних межах
    if (width <= 0 || width > 100 || height <= 0 || height > 100) {
        cerr << "Invalid level dimensions: " << width << "x" << height << endl;
        return false;
    }

    // Очищаємо існуючий рівень і створюємо новий
    levelData.clear();
    levelData.resize(height, std::vector<char>(width));

    // Зчитуємо дані рівня
    for (int i = 0; i < height; i++) {
        char rowBuffer[100]; // Достатній буфер
        inFile.read(rowBuffer, width);
        for (int j = 0; j < width; j++) {
            levelData[i][j] = rowBuffer[j];
        }
    }

    // Зчитуємо позицію гравця
    inFile.read(reinterpret_cast<char*>(&playerX), sizeof(playerX));
    inFile.read(reinterpret_cast<char*>(&playerY), sizeof(playerY));

    inFile.close();

    // Скидаємо стан гри при завантаженні нового рівня
    reset();

    cout << "Loaded level: " << width << "x" << height << ", player at (" << playerX << ", " << playerY << ")" << endl;
    return true;
}

void Level::createDefaultLevel() {
    cout << "Creating default level" << endl;
    width = 8;
    height = 9;

    levelData.clear();
    levelData.resize(height, std::vector<char>(width));

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

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            levelData[i][j] = defaultLevel[i][j];
        }
    }

    playerX = 1;
    playerY = 7;

    // Скидаємо стан гри при створенні нового рівня
    reset();

    cout << "Default level created" << endl;
}

bool Level::isObstacle(int x, int y) const {
    // Перевіряємо чи координати в межах поля і чи це не стіна
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return true; // За межами поля вважаємо перешкодою
    }
    return levelData[y][x] == WALL;
}

void Level::movePlayer(char direction) {
    if (isFinished || isFailed) return; // Якщо гра закінчена, рух неможливий

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
    default:
        return; // Невідомий напрямок - нічого не робимо
    }

    // Зберігаємо початкову позицію
    int startX = playerX;
    int startY = playerY;

    // Ковзаємо поки не зустрінемо перешкоду
    while (!isObstacle(newX + dx, newY + dy)) {
        newX += dx;
        newY += dy;

        // Додаємо клітинку до сліду
        Uint32 currentTime = SDL_GetTicks();
        trail.push_back({ newX, newY, currentTime });
    }

    // Оновлюємо позицію тільки якщо вона змінилась
    if (newX != playerX || newY != playerY) {
        playerX = newX;
        playerY = newY;

        // Перевіряємо умови перемоги чи поразки
        if (levelData[playerY][playerX] == TRAP) {
            isFailed = true;
            animationRadius = 0;
            lastAnimationTime = SDL_GetTicks();
            cout << "Player trapped! Game over!" << endl;
        }
        else if (levelData[playerY][playerX] == FINISH) {
            isFinished = true;
            animationRadius = 0;
            lastAnimationTime = SDL_GetTicks();
            cout << "Player reached finish! Level completed!" << endl;
        }
    }
}

void Level::reset() {
    isFinished = false;
    isFailed = false;
    animationRadius = 0;
    lastAnimationTime = 0;
    trail.clear();
    cout << "Level state reset" << endl;
}

void Level::updateAnimations(Uint32 currentTime) {
    // Оновлення сліду - видалення старих точок
    auto it = trail.begin();
    while (it != trail.end()) {
        if (currentTime - it->timeCreated > TRAIL_LIFETIME) {
            it = trail.erase(it);
        }
        else {
            ++it;
        }
    }

    // Оновлення анімації перемоги/поразки
    if ((isFinished || isFailed) && currentTime - lastAnimationTime >= ANIMATION_STEP_TIME) {
        animationRadius++;
        lastAnimationTime = currentTime;

        // Для діагностики - можна бачити прогрес анімації
        if (animationRadius % 5 == 0) {
            cout << "Animation radius: " << animationRadius << endl;
        }

        // Обмеження радіусу анімації, щоб не зростав нескінченно
        int maxRadius = (int)sqrt(width * width + height * height) + 1;
        if (animationRadius > maxRadius) {
            animationRadius = maxRadius;
        }
    }
}

int Level::getAnimationRadius() const {
    return animationRadius;
}