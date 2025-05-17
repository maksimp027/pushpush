#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include "renderer.h"
#include "level.h"

// Enumeration for selected brush types
enum BrushType {
    BRUSH_WALL,
    BRUSH_TRAP,
    BRUSH_START,
    BRUSH_FINISH,
    BRUSH_EMPTY
};

// Editor states
enum EditorState {
    DIMENSIONS_INPUT,
    LEVEL_EDITING
};

class LevelCreator {
private:
    Renderer& renderer;
    std::string savePath;

    // Editor state
    EditorState currentState;

    // Input field for dimensions
    std::string inputText;
    bool inputActive;

    // Level dimensions
    int mapWidth;
    int mapHeight;
    int cursorX;
    int cursorY;
    BrushType currentBrush;

    // Level data
    std::vector<std::vector<char>> levelData;
    bool hasStart;
    bool hasFinish;

    // Rendering and input handling
    void renderDimensionsInput();
    void renderCreationScreen();
    void handleInput(SDL_Event& e);
    void handleTextInput(SDL_Event& e);
    void paintCell(int x, int y);
    void switchBrush(BrushType brush);

    // Parse and validate dimensions
    bool parseDimensions();
    bool validateDimensions(int width, int height);

    // Level validation and saving
    bool validateLevel();
    bool saveLevel(const std::string& fileName);

public:
    LevelCreator(Renderer& renderer, const std::string& savePath);
    ~LevelCreator();

    bool initialize();
    void run();

    // Constants for level creation
    static const int MIN_SIZE = 6;
    static const int MAX_SIZE = 60;
    static const float MAX_RATIO;
};
