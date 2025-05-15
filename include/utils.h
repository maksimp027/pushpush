#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>

namespace utils {
    // Функція для видалення розширення файлу
    std::string removeFileExtension(const std::string& filename);

    // Функція для створення директорій, якщо вони не існують
    bool ensureDirectoryExists(const std::string& path);
}