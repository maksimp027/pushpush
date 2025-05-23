#pragma once

#include <SDL3/SDL.h>

// Константи для типів клітинок
const char WALL = 'x', EMPTY = '_', START = 's', FINISH = 'f', TRAP = 'd', PLAYER = 'P';

// Стани гри
enum GameState {
    MENU,
    LEVEL_SELECT,
    GAME_PLAYING,
    GAME_WIN,
    GAME_LOSE
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

// Часові константи для анімацій (у мілісекундах)
const int ANIMATION_STEP_TIME = 200;
const int TRAIL_LIFETIME = 200;