cmake_minimum_required(VERSION 3.16)
project(pushpush)

# Налаштування C++ стандарту
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Шляхи до вихідних файлів
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Include директорії
include_directories(
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/external/SDL3/include"
    "${PROJECT_SOURCE_DIR}/external/SDL3_ttf/include"
    "${PROJECT_SOURCE_DIR}/external/SDL3_image/include"
)

# Link директорії для бібліотек
link_directories(
    "${PROJECT_SOURCE_DIR}/external/SDL3/lib"
    "${PROJECT_SOURCE_DIR}/external/SDL3_ttf/lib"
    "${PROJECT_SOURCE_DIR}/external/SDL3_image/lib"
)

# Список вихідних файлів
set(SOURCES
    src/main.cpp
    src/game.cpp
    src/level.cpp
    src/renderer.cpp
    src/utils.cpp
)

# Список заголовочних файлів
set(HEADERS
    include/constants.h
    include/game.h
    include/level.h
    include/renderer.h
    include/utils.h
)

# Налаштування бібліотек SDL3
set(SDL3_LIB_DIR "${PROJECT_SOURCE_DIR}/external/SDL3/lib/x64")
set(SDL3_TTF_LIB_DIR "${PROJECT_SOURCE_DIR}/external/SDL3_ttf/lib/x64")
set(SDL3_IMAGE_LIB_DIR "${PROJECT_SOURCE_DIR}/external/SDL3_image/lib/x64")

# Створення виконуваного файлу
add_executable(pushpush ${SOURCES} ${HEADERS})

# Лінкування бібліотек SDL3
target_link_libraries(pushpush
    "${SDL3_LIB_DIR}/SDL3.lib"
    "${SDL3_TTF_LIB_DIR}/SDL3_ttf.lib"
    "${SDL3_IMAGE_LIB_DIR}/SDL3_image.lib"
)

# Для Windows: налаштування SDL_main
if(WIN32)
    target_compile_definitions(pushpush PRIVATE -DSDL_MAIN_HANDLED)
endif()

# Копіювання SDL DLL в директорію виконуваного файлу
add_custom_command(TARGET pushpush POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_LIB_DIR}/SDL3.dll"
        "$<TARGET_FILE_DIR:pushpush>/SDL3.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_TTF_LIB_DIR}/SDL3_ttf.dll"
        "$<TARGET_FILE_DIR:pushpush>/SDL3_ttf.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_IMAGE_LIB_DIR}/SDL3_image.dll"
        "$<TARGET_FILE_DIR:pushpush>/SDL3_image.dll"
)

# Створення директорії для ресурсів
add_custom_command(TARGET pushpush POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:pushpush>/assetst"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:pushpush>/assetst/levels"
)

# Копіювання шрифту
add_custom_command(TARGET pushpush POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/assetst/DroidSans.ttf"
        "$<TARGET_FILE_DIR:pushpush>/assetst/DroidSans.ttf"
)

# Копіювання рівнів, якщо вони існують
if(EXISTS "${PROJECT_SOURCE_DIR}/assetst/levels")
    add_custom_command(TARGET pushpush POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/assetst/levels"
            "$<TARGET_FILE_DIR:pushpush>/assetst/levels"
    )
endif()