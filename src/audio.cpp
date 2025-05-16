#include "audio.h"
#include <iostream>

AudioManager::AudioManager() : initialized(false), deviceID(0) {
    std::cout << "AudioManager created" << std::endl;
}

AudioManager::~AudioManager() {
    cleanup();
    std::cout << "AudioManager destroyed" << std::endl;
}

bool AudioManager::initialize() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL Audio initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Відкриваємо аудіо пристрій для відтворення
    deviceID = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (deviceID == 0) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return false;
    }

    // Відразу запускаємо пристрій (знімаємо з паузи)
    SDL_ResumeAudioDevice(deviceID);

    initialized = true;
    std::cout << "Audio system initialized successfully" << std::endl;
    return true;
}

void AudioManager::cleanup() {
    if (!initialized) return;

    // Видаляємо всі аудіо потоки
    for (auto& [name, stream] : audioStreams) {
        if (stream) {
            // Відв'язуємо потік від пристрою
            SDL_UnbindAudioStream(stream);
            // Знищуємо потік
            SDL_DestroyAudioStream(stream);
        }
    }
    audioStreams.clear();
    soundBuffers.clear();

    // Закриваємо аудіо пристрій
    if (deviceID != 0) {
        SDL_CloseAudioDevice(deviceID);
        deviceID = 0;
    }

    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    initialized = false;
    std::cout << "Audio system cleaned up" << std::endl;
}

bool AudioManager::loadSound(const std::string& name, const std::string& filePath) {
    if (!initialized) {
        std::cerr << "Audio system not initialized" << std::endl;
        return false;
    }

    // Перевіряємо, чи звук вже завантажено
    if (audioStreams.find(name) != audioStreams.end()) {
        std::cout << "Sound '" << name << "' already loaded" << std::endl;
        return true;
    }

    // Завантажуємо WAV файл
    SDL_AudioSpec spec;
    Uint8* buffer = nullptr;
    Uint32 length = 0;

    if (SDL_LoadWAV(filePath.c_str(), &spec, &buffer, &length) == NULL) {
        std::cerr << "Failed to load WAV file '" << filePath << "': " << SDL_GetError() << std::endl;
        return false;
    }

    // Отримуємо спеціфікацію пристрою
    SDL_AudioSpec deviceSpec;
    int sample_frames = 0;
    if (SDL_GetAudioDeviceFormat(deviceID, &deviceSpec, &sample_frames) < 0) {
        std::cerr << "Failed to get audio device format: " << SDL_GetError() << std::endl;
        SDL_free(buffer);
        return false;
    }

    // Створюємо аудіо потік з формату WAV до формату пристрою
    SDL_AudioStream* stream = SDL_CreateAudioStream(&spec, &deviceSpec);
    if (!stream) {
        std::cerr << "Failed to create audio stream: " << SDL_GetError() << std::endl;
        SDL_free(buffer);
        return false;
    }

    // Зберігаємо буфер даних
    std::vector<Uint8> soundData(buffer, buffer + length);
    soundBuffers[name] = soundData;

    // Зберігаємо потік
    audioStreams[name] = stream;

    // Звільняємо буфер WAV
    SDL_free(buffer);

    std::cout << "Sound '" << name << "' loaded successfully" << std::endl;
    return true;
}

void AudioManager::playSound(const std::string& name) {
    if (!initialized) {
        std::cerr << "Audio system not initialized" << std::endl;
        return;
    }

    // Перевіряємо, чи звук завантажено
    auto streamIt = audioStreams.find(name);
    auto bufferIt = soundBuffers.find(name);

    if (streamIt == audioStreams.end() || bufferIt == soundBuffers.end()) {
        std::cerr << "Sound '" << name << "' not loaded" << std::endl;
        return;
    }

    SDL_AudioStream* stream = streamIt->second;
    const std::vector<Uint8>& buffer = bufferIt->second;

    // Очищаємо потік перед додаванням нових даних
    SDL_ClearAudioStream(stream);

    // Додаємо дані у потік
    if (SDL_PutAudioStreamData(stream, buffer.data(), buffer.size()) < 0) {
        std::cerr << "Failed to put data into audio stream: " << SDL_GetError() << std::endl;
        return;
    }

    // Прив'язуємо потік до пристрою
    if (SDL_BindAudioStream(deviceID, stream) < 0) {
        std::cerr << "Failed to bind audio stream to device: " << SDL_GetError() << std::endl;
        return;
    }

    std::cout << "Playing sound '" << name << "'" << std::endl;
}