# SDL3_ttf CMake configuration file:
# This file is meant to be placed in a cmake subfolder of SDL3_ttf-devel-3.1.0-VC.zip

include(FeatureSummary)
set_package_properties(SDL3_ttf PROPERTIES
    URL "https://www.libsdl.org/projects/SDL_ttf/"
    DESCRIPTION "Support for TrueType (.ttf) font files with Simple Directmedia Layer"
)

cmake_minimum_required(VERSION 3.0...3.28)

set(SDL3_ttf_FOUND TRUE)

set(SDLTTF_VENDORED TRUE)

set(SDLTTF_HARFBUZZ TRUE)
set(SDLTTF_FREETYPE TRUE)


if(SDL_CPU_X86)
    set(_sdl3_ttf_arch_subdir "x86")
elseif(SDL_CPU_X64 OR SDL_CPU_ARM64EC)
    set(_sdl3_ttf_arch_subdir "x64")
elseif(SDL_CPU_ARM64)
    set(_sdl3_ttf_arch_subdir "arm64")
else()
    set(SDL3_mixer_FOUND FALSE)
    return()
endif()

set(_sdl3_ttf_incdir       "${CMAKE_CURRENT_LIST_DIR}/../include")
set(_sdl3_ttf_library      "${CMAKE_CURRENT_LIST_DIR}/../lib/${_sdl3_ttf_arch_subdir}/SDL3_ttf.lib")
set(_sdl3_ttf_dll          "${CMAKE_CURRENT_LIST_DIR}/../lib/${_sdl3_ttf_arch_subdir}/SDL3_ttf.dll")

# All targets are created, even when some might not be requested though COMPONENTS.
# This is done for compatibility with CMake generated SDL3_ttf-target.cmake files.

set(SDL3_ttf_SDL3_ttf-shared_FOUND TRUE)
if(NOT TARGET SDL3_ttf::SDL3_ttf-shared)
    add_library(SDL3_ttf::SDL3_ttf-shared SHARED IMPORTED)
    set_target_properties(SDL3_ttf::SDL3_ttf-shared
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${_sdl3_ttf_incdir}"
            IMPORTED_IMPLIB "${_sdl3_ttf_library}"
            IMPORTED_LOCATION "${_sdl3_ttf_dll}"
            COMPATIBLE_INTERFACE_BOOL "SDL3_SHARED"
            INTERFACE_SDL3_SHARED "ON"
    )
endif()

unset(_sdl3_ttf_arch_subdir)
unset(_sdl3_ttf_incdir)
unset(_sdl3_ttf_library)
unset(_sdl3_ttf_dll)

set(SDL3_ttf_SDL3_ttf-static_FOUND TRUE)

set(SDL3_ttf_SDL3_ttf_FOUND FALSE)
if(SDL3_ttf_SDL3_ttf-hared_FOUND OR SDL3_ttf_SDL3_ttf-static_FOUND)
    set(SDL3_ttf_SDL3_ttf_FOUND TRUE)
endif()

function(_sdl_create_target_alias_compat NEW_TARGET TARGET)
    if(CMAKE_VERSION VERSION_LESS "3.18")
        # Aliasing local targets is not supported on CMake < 3.18, so make it global.
        add_library(${NEW_TARGET} INTERFACE IMPORTED)
        set_target_properties(${NEW_TARGET} PROPERTIES INTERFACE_LINK_LIBRARIES "${TARGET}")
    else()
        add_library(${NEW_TARGET} ALIAS ${TARGET})
    endif()
endfunction()

# Make sure SDL3_ttf::SDL3_ttf always exists
if(NOT TARGET SDL3_ttf::SDL3_ttf)
    if(TARGET SDL3_ttf::SDL3_ttf-shared)
        _sdl_create_target_alias_compat(SDL3_ttf::SDL3_ttf SDL3_ttf::SDL3_ttf-shared)
    endif()
endif()

check_required_components(SDL3_ttf)
