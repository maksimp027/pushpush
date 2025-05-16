#pragma once

#include "constants.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <SDL3/SDL.h>

class Level {
public:
    // Структура для зберігання точок сліду гравця
    struct TrailPoint {
        int x, y;
        Uint32 timeCreated;
    };

private:
    // Дані рівня
    int width;
    int height;
    std::vector<std::vector<char>> levelData;
    int playerX;
    int playerY;
    std::string levelsPath;

    // Змінні для стану гри
    bool isFinished;
    bool isFailed;

    // Змінні для анімації
    int animationRadius;
    Uint32 lastAnimationTime;

    // Змінні для сліду гравця
    std::vector<TrailPoint> trail;

public:
    // Конструктор і деструктор
    Level(const std::string& levelsDirectory);
    ~Level();

    // Властивості рівня
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getPlayerX() const { return playerX; }
    int getPlayerY() const { return playerY; }
    char getTileAt(int x, int y) const;

    // Стан гри
    bool isLevelFinished() const { return isFinished; }
    bool isLevelFailed() const { return isFailed; }
    void reset();

    // Операції з файлами рівнів
    std::vector<std::string> getLevelFileList();
    bool loadLevelFromFile(const std::string& filename);
    void createDefaultLevel();

    // Ігрова логіка
    bool isObstacle(int x, int y) const;
    void movePlayer(char direction);

    // Методи для анімацій
    void updateAnimations(Uint32 currentTime);
    int getAnimationRadius() const;
    const std::vector<TrailPoint>& getTrail() const { return trail; }
};