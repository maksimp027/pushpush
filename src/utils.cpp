#include "utils.h"
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

namespace utils {
    // Функція для видалення розширення файлу
    std::string removeFileExtension(const std::string& filename) {
        size_t lastDotPos = filename.find_last_of(".");
        if (lastDotPos == std::string::npos) {
            return filename;
        }
        return filename.substr(0, lastDotPos);
    }

    // Функція для створення директорій, якщо вони не існують
    bool ensureDirectoryExists(const std::string& path) {
        try {
            if (!fs::exists(path)) {
                return fs::create_directories(path);
            }
            return true;
        }
        catch (const fs::filesystem_error& e) {
            cerr << "Error creating directory: " << e.what() << endl;
            return false;
        }
    }
}