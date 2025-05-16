#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <map>
#include <vector>

class AudioManager {
private:
    bool initialized;
    SDL_AudioDeviceID deviceID;
    std::map<std::string, SDL_AudioStream*> audioStreams;
    std::map<std::string, std::vector<Uint8>> soundBuffers;

public:
    AudioManager();
    ~AudioManager();

    bool initialize();
    void cleanup();

    bool loadSound(const std::string& name, const std::string& filePath);
    void playSound(const std::string& name);
};