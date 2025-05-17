#include "creator.h"
#include "constants.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <sstream>

namespace fs = std::filesystem;

const float LevelCreator::MAX_RATIO = 2.0f;

LevelCreator::LevelCreator(Renderer& renderer, const std::string& savePath)
    : renderer(renderer),
    savePath(savePath),
    currentState(DIMENSIONS_INPUT),
    inputText(""),
    inputActive(true),
    mapWidth(0),
    mapHeight(0),
    cursorX(0),
    cursorY(0),
    currentBrush(BRUSH_WALL),
    hasStart(false),
    hasFinish(false) {

    std::cout << "Level Creator initialized" << std::endl;
}

LevelCreator::~LevelCreator() {
    std::cout << "Level Creator destroyed" << std::endl;
}

bool LevelCreator::initialize() {
    currentState = DIMENSIONS_INPUT;
    inputText = "";
    inputActive = true;
    SDL_StartTextInput(renderer.getWindow());
    return true;
}

void LevelCreator::run() {
    bool running = true;
    bool saved = false;

    std::cout << "Level Creator started" << std::endl;

    while (running) {
        // Handle SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                break;
            }

            if (currentState == DIMENSIONS_INPUT) {
                if (event.type == SDL_EVENT_TEXT_INPUT) {
                    // Accept only digits and '*'
                    std::string text = event.text.text;
                    for (char c : text) {
                        if (std::isdigit(static_cast<unsigned char>(c)) || c == '*') {
                            inputText += c;
                        }
                    }
                }
                else if (event.type == SDL_EVENT_KEY_DOWN) {
                    switch (event.key.key) {
                    case SDLK_BACKSPACE:
                        if (!inputText.empty()) inputText.pop_back();
                        break;
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_RETURN:
                        if (parseDimensions()) {
                            // Initialize the level data with the parsed dimensions
                            levelData.resize(mapHeight, std::vector<char>(mapWidth, EMPTY));
                            for (int y = 0; y < mapHeight; y++)
                                for (int x = 0; x < mapWidth; x++)
                                    if (x == 0 || y == 0 || x == mapWidth - 1 || y == mapHeight - 1)
                                        levelData[y][x] = WALL;
                            cursorX = mapWidth / 2;
                            cursorY = mapHeight / 2;
                            renderer.calculateScaling(mapWidth, mapHeight);
                            currentState = LEVEL_EDITING;
                            SDL_StopTextInput(renderer.getWindow());
                        }
                        break;
                    }
                }
            }
            else if (currentState == LEVEL_EDITING) {
                if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (event.key.key == SDLK_ESCAPE) {
                        running = false;
                        break;
                    }
                    if (event.key.key == SDLK_RETURN) {
                        if (validateLevel()) {
                            std::string fileName;
                            std::cout << "Enter level file name (without extension): ";
                            std::getline(std::cin, fileName);
                            if (saveLevel(fileName)) {
                                std::cout << "Level saved successfully!" << std::endl;
                                saved = true;
                                running = false;
                            }
                        }
                    }
                    else {
                        handleInput(event);
                    }
                }
            }
        }


        // Render based on current state
        if (currentState == DIMENSIONS_INPUT) {
            renderDimensionsInput();
        }
        else if (currentState == LEVEL_EDITING) {
            renderCreationScreen();
        }

        // Small delay to avoid hogging CPU
        SDL_Delay(16);
    }

    // Disable text input when done
    SDL_StopTextInput(renderer.getWindow());
    if (!saved) {
        std::cout << "Level creation cancelled." << std::endl;
    }
}

void LevelCreator::renderDimensionsInput() {
    // Set background color
    SDL_SetRenderDrawColor(renderer.getRenderer(), 50, 50, 50, 255);
    SDL_RenderClear(renderer.getRenderer());

    // Get window dimensions
    int windowWidth, windowHeight;
    SDL_GetWindowSize(renderer.getWindow(), &windowWidth, &windowHeight);

    // Draw title
    SDL_Color titleColor = { 255, 255, 255, 255 };
    renderer.renderText("Create New Level", windowWidth / 2 - 150, 100, titleColor, renderer.getSmallFont());

    // Draw input prompt
    renderer.renderText("Enter the dimensions of the level in format A*B:", windowWidth / 2 - 200, 180, titleColor, renderer.getSmallFont());

    // Draw input field background
    SDL_FRect inputRect = {
        (float)(windowWidth / 2 - 100),
        220,
        200,
        40
    };
    SDL_SetRenderDrawColor(renderer.getRenderer(), 20, 20, 20, 255);
    SDL_RenderFillRect(renderer.getRenderer(), &inputRect);

    // Draw border for input field
    SDL_SetRenderDrawColor(renderer.getRenderer(), 200, 200, 200, 255);
    SDL_RenderRect(renderer.getRenderer(), &inputRect);

    // Draw input text
    std::string displayText = inputText;
    if (SDL_GetTicks() % 1000 < 500 && inputActive) {
        displayText += "|"; // Blinking cursor
    }
    renderer.renderText(displayText.empty() ? " " : displayText, windowWidth / 2 - 90, 230, titleColor, renderer.getSmallFont());


    // Draw validation constraints
    SDL_Color constraintColor = { 200, 200, 120, 255 };
    renderer.renderText("Dimensions can only be from 6*6 to 60*60", windowWidth / 2 - 180, 300, constraintColor, renderer.getSmallFont());
    renderer.renderText("Proportions can be from 1/2 to 2/1", windowWidth / 2 - 160, 330, constraintColor, renderer.getSmallFont());

    // Draw action instructions
    SDL_Color instructionsColor = { 150, 150, 150, 255 };
    renderer.renderText("Press Enter to continue, Esc to cancel", windowWidth / 2 - 150, 400, instructionsColor, renderer.getSmallFont());

    // Present the render
    SDL_RenderPresent(renderer.getRenderer());
}

void LevelCreator::handleTextInput(SDL_Event& e) {
    if (e.type == SDL_EVENT_KEY_DOWN) {
        switch (e.key.key) {
        case SDLK_BACKSPACE:
            if (!inputText.empty()) {
                inputText.pop_back();
            }
            break;

        case SDLK_RETURN:
            if (parseDimensions()) {
                // Initialize the level data with the parsed dimensions
                levelData.resize(mapHeight, std::vector<char>(mapWidth, EMPTY));

                // Add walls around the perimeter
                for (int y = 0; y < mapHeight; y++) {
                    for (int x = 0; x < mapWidth; x++) {
                        if (x == 0 || y == 0 || x == mapWidth - 1 || y == mapHeight - 1) {
                            levelData[y][x] = WALL;
                        }
                    }
                }

                // Initialize cursor position
                cursorX = mapWidth / 2;
                cursorY = mapHeight / 2;

                // Configure renderer for the level creator
                renderer.calculateScaling(mapWidth, mapHeight);

                // Switch to level editing mode
                currentState = LEVEL_EDITING;
                SDL_StopTextInput(NULL); // <--- Place this line here
            }
            break;

        }
    }
}

bool LevelCreator::parseDimensions() {
    // Try to match the pattern A*B
    std::regex dimRegex("(\\d+)\\*(\\d+)");
    std::smatch matches;

    if (std::regex_match(inputText, matches, dimRegex)) {
        // Extract width and height
        int width = std::stoi(matches[1]);
        int height = std::stoi(matches[2]);

        // Validate the dimensions
        if (validateDimensions(width, height)) {
            mapWidth = width;
            mapHeight = height;
            return true;
        }
    }

    return false;
}

bool LevelCreator::validateDimensions(int width, int height) {
    // Check size constraints
    if (width < MIN_SIZE || width > MAX_SIZE ||
        height < MIN_SIZE || height > MAX_SIZE) {
        return false;
    }

    // Check aspect ratio
    float ratio = static_cast<float>(width) / height;
    if (ratio > MAX_RATIO || ratio < 1.0f / MAX_RATIO) {
        return false;
    }

    return true;
}

void LevelCreator::renderCreationScreen() {
    // Set background color
    SDL_SetRenderDrawColor(renderer.getRenderer(), 50, 50, 50, 255);
    SDL_RenderClear(renderer.getRenderer());

    // Get window and cell dimensions
    int windowWidth, windowHeight;
    SDL_GetWindowSize(renderer.getWindow(), &windowWidth, &windowHeight);

    float cellSize = renderer.getCellSize();
    float offsetX = renderer.getOffsetX();
    float offsetY = renderer.getOffsetY();

    // Draw the level grid
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            SDL_FRect cellRect = {
                offsetX + x * cellSize,
                offsetY + y * cellSize,
                cellSize,
                cellSize
            };

            // Draw cell based on its type
            switch (levelData[y][x]) {
            case WALL:
                SDL_SetRenderDrawColor(renderer.getRenderer(), 150, 150, 150, 255);
                break;
            case TRAP:
                SDL_SetRenderDrawColor(renderer.getRenderer(), 196, 36, 44, 255);
                break;
            case START:
                SDL_SetRenderDrawColor(renderer.getRenderer(), 252, 252, 177, 255);
                break;
            case FINISH:
                SDL_SetRenderDrawColor(renderer.getRenderer(), 42, 166, 220, 255);
                break;
            default:
                SDL_SetRenderDrawColor(renderer.getRenderer(), 244, 244, 240, 255);
                break;
            }

            SDL_RenderFillRect(renderer.getRenderer(), &cellRect);

            // Draw grid lines
            SDL_SetRenderDrawColor(renderer.getRenderer(), 30, 30, 30, 255);
            SDL_RenderRect(renderer.getRenderer(), &cellRect);
        }
    }

    // Draw the cursor
    SDL_FRect cursorRect = {
        offsetX + cursorX * cellSize,
        offsetY + cursorY * cellSize,
        cellSize,
        cellSize
    };

    // Draw cursor highlight
    SDL_SetRenderDrawColor(renderer.getRenderer(), 255, 255, 255, 100);
    SDL_RenderFillRect(renderer.getRenderer(), &cursorRect);

    // Draw current brush indicator
    std::string brushName;
    SDL_Color brushColor;

    switch (currentBrush) {
    case BRUSH_WALL:
        brushName = "Wall";
        brushColor = { 150, 150, 150, 255 };
        break;
    case BRUSH_TRAP:
        brushName = "Trap";
        brushColor = { 196, 36, 44, 255 };
        break;
    case BRUSH_START:
        brushName = "Start";
        brushColor = { 252, 252, 177, 255 };
        break;
    case BRUSH_FINISH:
        brushName = "Finish";
        brushColor = { 42, 166, 220, 255 };
        break;
    case BRUSH_EMPTY:
        brushName = "Eraser";
        brushColor = { 244, 244, 240, 255 };
        break;
    }

    // Draw current brush indicator
    SDL_FRect brushIndicator = {
        20.0f,
        (float)(windowHeight - 80),
        30.0f,
        30.0f
    };

    SDL_SetRenderDrawColor(renderer.getRenderer(), brushColor.r, brushColor.g, brushColor.b, brushColor.a);
    SDL_RenderFillRect(renderer.getRenderer(), &brushIndicator);
    SDL_SetRenderDrawColor(renderer.getRenderer(), 255, 255, 255, 255);
    SDL_RenderRect(renderer.getRenderer(), &brushIndicator);

    // Draw brush text
    std::string brushText = "Current: ";
    renderer.renderText(brushText, 60, windowHeight - 70, { 255, 255, 255, 255 }, renderer.getSmallFont());
    renderer.renderText(brushName, 140, windowHeight - 70, brushColor, renderer.getSmallFont());

    // Draw help text
    renderer.renderText("WASD - Move | Space - Paint | C - Change Brush | Enter - Save | Esc - Cancel",
        20, windowHeight - 30, { 255, 255, 255, 255 }, renderer.getSmallFont());

    // Render everything
    SDL_RenderPresent(renderer.getRenderer());
}

void LevelCreator::handleInput(SDL_Event& e) {
    if (e.type == SDL_EVENT_KEY_DOWN) {
        switch (e.key.key) {
        case SDLK_W: case SDLK_UP:
            cursorY = std::max(0, cursorY - 1);
            break;

        case SDLK_S: case SDLK_DOWN:
            cursorY = std::min(mapHeight - 1, cursorY + 1);
            break;

        case SDLK_A: case SDLK_LEFT:
            cursorX = std::max(0, cursorX - 1);
            break;

        case SDLK_D: case SDLK_RIGHT:
            cursorX = std::min(mapWidth - 1, cursorX + 1);
            break;

        case SDLK_SPACE:
            paintCell(cursorX, cursorY);
            break;

        case SDLK_C:
            // Cycle through brush types
            switch (currentBrush) {
            case BRUSH_WALL:
                switchBrush(BRUSH_TRAP);
                break;
            case BRUSH_TRAP:
                switchBrush(BRUSH_START);
                break;
            case BRUSH_START:
                switchBrush(BRUSH_FINISH);
                break;
            case BRUSH_FINISH:
                switchBrush(BRUSH_EMPTY);
                break;
            case BRUSH_EMPTY:
                switchBrush(BRUSH_WALL);
                break;
            }
            break;
        }
    }
}

void LevelCreator::paintCell(int x, int y) {
    // Don't allow editing the border
    if (x == 0 || y == 0 || x == mapWidth - 1 || y == mapHeight - 1) {
        return;
    }

    char newTile = EMPTY;
    switch (currentBrush) {
    case BRUSH_WALL:
        newTile = WALL;
        break;

    case BRUSH_TRAP:
        newTile = TRAP;
        break;

    case BRUSH_START:
        // Remove existing start position if there is one
        if (hasStart) {
            for (int cy = 0; cy < mapHeight; cy++) {
                for (int cx = 0; cx < mapWidth; cx++) {
                    if (levelData[cy][cx] == START) {
                        levelData[cy][cx] = EMPTY;
                    }
                }
            }
        }
        newTile = START;
        hasStart = true;
        break;

    case BRUSH_FINISH:
        // Remove existing finish position if there is one
        if (hasFinish) {
            for (int cy = 0; cy < mapHeight; cy++) {
                for (int cx = 0; cx < mapWidth; cx++) {
                    if (levelData[cy][cx] == FINISH) {
                        levelData[cy][cx] = EMPTY;
                    }
                }
            }
        }
        newTile = FINISH;
        hasFinish = true;
        break;

    case BRUSH_EMPTY:
        // Check if we're removing a special tile
        if (levelData[y][x] == START) hasStart = false;
        if (levelData[y][x] == FINISH) hasFinish = false;
        newTile = EMPTY;
        break;
    }

    levelData[y][x] = newTile;
}

void LevelCreator::switchBrush(BrushType brush) {
    currentBrush = brush;
}

bool LevelCreator::validateLevel() {
    if (!hasStart) {
        std::cout << "Error: Level must have a start position." << std::endl;
        return false;
    }

    if (!hasFinish) {
        std::cout << "Error: Level must have a finish position." << std::endl;
        return false;
    }

    return true;
}

bool LevelCreator::saveLevel(const std::string& fileName) {
    // Create directories if they don't exist
    try {
        if (!fs::exists(savePath)) {
            fs::create_directories(savePath);
            std::cout << "Created directory: " << savePath << std::endl;
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
        return false;
    }

    // Find player start position
    int playerX = -1, playerY = -1;
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            if (levelData[y][x] == START) {
                playerX = x;
                playerY = y;
                break;
            }
        }
        if (playerX != -1) break;
    }

    // Create full path to file
    std::string filePath = savePath + "/" + fileName + ".bin";

    // Open file for binary writing
    std::ofstream outFile(filePath, std::ios::binary | std::ios::out);

    if (!outFile) {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
        return false;
    }

    // Write level dimensions
    outFile.write(reinterpret_cast<const char*>(&mapWidth), sizeof(mapWidth));
    outFile.write(reinterpret_cast<const char*>(&mapHeight), sizeof(mapHeight));

    // Write level data
    for (int i = 0; i < mapHeight; i++) {
        outFile.write(levelData[i].data(), mapWidth);
    }

    // Write player position
    outFile.write(reinterpret_cast<const char*>(&playerX), sizeof(playerX));
    outFile.write(reinterpret_cast<const char*>(&playerY), sizeof(playerY));

    outFile.close();

    std::cout << "Level successfully saved to " << filePath << std::endl;
    return true;
}
