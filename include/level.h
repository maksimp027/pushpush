#pragma once

#include "constants.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

class Level {
private:
    int width;
    int height;
    std::vector<std::vector<char>> levelData;
    int playerX;
    int playerY;
    std::string levelsPath;

public:
    Level(const std::string& levelsDirectory);
    ~Level();

    // Властивості рівня
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getPlayerX() const { return playerX; }
    int getPlayerY() const { return playerY; }
    char getTileAt(int x, int y) const;
    
    // Операції з файлами рівнів
    std::vector<std::string> getLevelFileList();
    bool loadLevelFromFile(const std::string& filename);
    void createDefaultLevel();
    
    // Ігрова логіка
    bool isObstacle(int x, int y) const;
    void movePlayer(char direction);
};