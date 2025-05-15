#pragma once

#include <SDL3/SDL.h>

// Константи для типів клітинок
const char WALL = 'x', EMPTY = '_', START = 's', FINISH = 'f', TRAP = 'd', PLAYER = 'P';

// Стани гри
enum GameState {
    MENU,
    LEVEL_SELECT,
    GAME_PLAYING
};

// Розміри вікна
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 900;

// Фіксована висота ігрового поля
const int GAME_FIELD_HEIGHT = 600;

// Максимальна кількість видимих рівнів
const int MAX_VISIBLE_LEVELS = 10;

// Кількість елементів меню
const int MENU_ITEMS = 4;