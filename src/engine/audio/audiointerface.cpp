/*
* Copyright 2021 Conquer Space
*/
#include "engine/audio/audiointerface.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <stb_vorbis.h>

#include <chrono>
#include <string>

namespace conquerspace::engine::audio {
class ALAudioAsset : public AudioAsset {
 public:
    ALAudioAsset() {
        // Make audio things
        alGenBuffers(1, &buffer);
        alGenSources((ALuint)1, &source);

        alSourcef(source, AL_PITCH, 1);
        alSourcef(source, AL_GAIN, 1);

        alSource3f(source, AL_POSITION, 0, 0, 0);
        alSource3f(source, AL_VELOCITY, 0, 0, 0);
        alSourcei(source, AL_LOOPING, AL_FALSE);
    }

    void SetGain(float gain) {
        alSourcef(source, AL_GAIN, gain);
    }

    void SetPitch(float pitch) {
        alSourcef(source, AL_PITCH, pitch);
    }

    void SetLooping(bool looping) {
        alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
    }

    void Play() {
        alSourcePlay(source);
    }

    void Stop() {
        alSourceStop(source);
    }

    void Resume() {
        alSourcePlay(source);
    }

    void Pause() {
        alSourcePause(source);
    }

    void Rewind() {
        alSourceRewind(source);
    }

    bool IsPlaying() {
        ALint source_state;
        alGetSourcei(source, AL_SOURCE_STATE, &source_state);
        return (source_state == AL_PLAYING);
    }

    float PlayPosition() {
        float length;
        alGetSourcef(source, AL_SEC_OFFSET, &length);
        return length;
    }

    /**
     * Length in seconds.
     */
    float Length() {
        return length;
    }

    ALuint source;
    ALuint buffer;
    float length = 0;

    ~ALAudioAsset() {
        alDeleteSources(1, &source);
        alDeleteBuffers(1, &buffer);
    }
};
}  // namespace conquerspace::engine::audio

using conquerspace::engine::audio::AudioInterface;
using conquerspace::engine::audio::AudioAsset;

AudioInterface::AudioInterface() {
    logger = spdlog::stdout_color_mt("audio");
}

void AudioInterface::Initialize() {
    SPDLOG_LOGGER_INFO(logger, "Intializing OpenAL");
    InitALContext();
    PrintInformation();
    InitListener();

    // Load playlist
    std::ifstream playlist_input = std::ifstream("../data/core/music/music_list.hjson");
    Hjson::DecoderOptions decOpt;
    playlist_input >> Hjson::StreamDecoder(playlist, decOpt);
}

void AudioInterface::Pause(bool to_pause) {}

void AudioInterface::PauseMusic(bool to_pause) {}

std::string AudioInterface::GetAudioVersion() {
    return std::string();
}

void AudioInterface::Destruct() {
    to_quit = true;
    worker_thread.join();
    SPDLOG_LOGGER_INFO(logger, "Killed OpenAL");
    // Clear sources and buffers
    music.reset();
    // Kill off music
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

void AudioInterface::StartWorker() {
    to_quit = false;
    // Worker thread
    worker_thread = std::thread([&]() {
        // Play music, handle playlist
        // Quickly inserted random algorithm to make linter happy.
        int selected_track = (time(0) * 0x0000BC8F) % 0x7FFFFFFF % playlist.size();
        while (!to_quit) {
            // Choose random song from thing
            selected_track++;
            selected_track %= playlist.size();
            Hjson::Value track_info = playlist[selected_track];
            std::string track_file = "../data/core/music/" + track_info["file"];
            SPDLOG_LOGGER_INFO(logger, "Loading track \'{}\'", track_info["name"]);
            auto mfile = std::ifstream(track_file, std::ios::binary);
            if (mfile.good()) {
                music = LoadOgg(mfile);
                SPDLOG_LOGGER_INFO(logger, "Length of audio: {}", music->Length());
            } else {
                SPDLOG_LOGGER_ERROR(logger, "Failed to load audio {}", track_info["name"]);
            }

            if (music != nullptr) {
                music->SetGain(music_volume);
                music->Play();
                while (music->IsPlaying() && !to_quit) {
                    // Wait
                }
                // Stop music
                music->Stop();
                music.reset();
                SPDLOG_LOGGER_INFO(logger, "Completed track");
            }
        }
    });
}

void AudioInterface::RequestPlayAudio() {}

void AudioInterface::SetMusicVolume(float volume) {
    if (music != nullptr) {
        music->SetGain(volume);
    }
    music_volume = volume;
}

inline bool isBigEndian() {
    int a = 1;
    return !(reinterpret_cast<char*>(&a))[0];
}

inline int convertToInt(char* buffer, int len) {
    int a = 0;
    if (!isBigEndian()) {
        for (int i = 0; i < len; i++)
            (reinterpret_cast<char*>(&a))[i] = buffer[i];
    } else {
        for (int i = 0; i < len; i++)
            (reinterpret_cast<char*>(&a))[3 - i] = buffer[i];
    }
    return a;
}

inline ALenum to_al_format(int16 channels, int16 samples) {
    bool stereo = (channels > 1);

    switch (samples) {
    case 16:
        return (stereo) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    case 8:
        return (stereo) ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
    default:
        return -1;
    }
}

std::unique_ptr<AudioAsset> AudioInterface::LoadWav(std::ifstream &in) {
    auto audio_asset = std::make_unique<ALAudioAsset>();
    char buffer[4];
    in.read(buffer, 4);
    if (strncmp(buffer, "RIFF", 4) != 0) {
        // Not a valid file
    }
    in.read(buffer, 4);
    in.read(buffer, 4);      // WAVE
    in.read(buffer, 4);      // fmt
    in.read(buffer, 4);      // 16
    in.read(buffer, 2);      // 1
    in.read(buffer, 2);
    int channels = convertToInt(buffer, 2);
    in.read(buffer, 4);
    int samplerate = convertToInt(buffer, 4);
    in.read(buffer, 4);
    in.read(buffer, 2);
    in.read(buffer, 2);
    int frequency = convertToInt(buffer, 2);
    in.read(buffer, 4);      // data
    in.read(buffer, 4);
    int size = convertToInt(buffer, 4);
    char* data = new char[size];
    in.read(data, size);

    ALenum format = to_al_format(channels, frequency);

    // Copy buffer
    alBufferData(audio_asset->buffer, format, data, size, samplerate);
    alSourcei(audio_asset->source, AL_BUFFER, audio_asset->buffer);

    delete[] data;
    return audio_asset;
}

std::unique_ptr<AudioAsset> AudioInterface::LoadOgg(std::ifstream& input) {
    // Read file
    input.seekg(0, std::ios::end);
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);
    char* data = new char[size];
    input.read(data, size);

    auto audio_asset = std::make_unique<ALAudioAsset>();
    int16* buffer;
    int channels;
    int sample_rate;
    int numSamples = stb_vorbis_decode_memory((unsigned char*)(data),
                                size, &channels, &sample_rate, &buffer);

    ALenum format = channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    alBufferData(audio_asset->buffer, format, buffer,
                                numSamples * channels * sizeof(int16), sample_rate);
    if (alGetError() != ALC_NO_ERROR) {
        spdlog::info("{}", alGetError());
    }
    audio_asset->length = static_cast<float>(numSamples) / static_cast<float>(sample_rate);
    alSourcei(audio_asset->source, AL_BUFFER, audio_asset->buffer);

    // Free memory
    delete[] data;
    free(buffer);
    return audio_asset;
}

void AudioInterface::PrintInformation() {
    // OpenAL information
    SPDLOG_LOGGER_INFO(logger, "OpenAL version: {}", alGetString(AL_VERSION));
    SPDLOG_LOGGER_INFO(logger, "OpenAL renderer: {}", alGetString(AL_RENDERER));
    SPDLOG_LOGGER_INFO(logger, "OpenAL vendor: {}", alGetString(AL_VENDOR));
}

void AudioInterface::InitListener() {
    // Set listener
    alListener3f(AL_POSITION, 0, 0, -0);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
    alListenerfv(AL_ORIENTATION, listenerOri);
}

void AudioInterface::InitALContext() {
    // Init devices
    enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
    if (enumeration == AL_FALSE) {
        SPDLOG_LOGGER_ERROR(logger, "Enumeration extension not available");
    }

    const ALCchar* defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

    device = alcOpenDevice(defaultDeviceName);
    if (!device) {
        SPDLOG_LOGGER_ERROR(logger, "Unable to open default device");
    }

    context = alcCreateContext(device, NULL);
    if (!alcMakeContextCurrent(context)) {
        SPDLOG_LOGGER_ERROR(logger, "Failed to make default context");
    }
}