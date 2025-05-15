#include "level.h"
#include "constants.h"
#include "utils.h"
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

Level::Level(const std::string& levelsDirectory) 
    : width(0), height(0), playerX(0), playerY(0), levelsPath(levelsDirectory) {
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
    
    return levelFiles;
}

bool Level::loadLevelFromFile(const std::string& filename) {
    std::string filePath = levelsPath + "\\" + filename;

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

    cout << "Loaded level: " << width << "x" << height << ", player at (" << playerX << ", " << playerY << ")" << endl;
    return true;
}

void Level::createDefaultLevel() {
    width = 8;
    height = 9;

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

    cout << "Using default level" << endl;
}

bool Level::isObstacle(int x, int y) const {
    // Перевіряємо чи координати в межах поля і чи це не стіна
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return true; // За межами поля вважаємо перешкодою
    }
    return levelData[y][x] == WALL;
}

void Level::movePlayer(char direction) {
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
    }
}