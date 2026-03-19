#include "FoxRenderer.h"
#include "VFX/Audio/AudioManager.h"

#define MINIAUDIO_IMPLEMENTATION
#include "VFX/Audio/miniaudio.h"

// Static member definitions
std::unique_ptr<Fox::Audio::AudioManager> Fox::Core::Singleton<Fox::Audio::AudioManager>::instance = nullptr;
std::once_flag Fox::Core::Singleton<Fox::Audio::AudioManager>::initFlag;

namespace Fox {
	
	namespace Audio {

        class AudioEngine {
        private:
            ma_engine engine{};

        public:
            AudioEngine() {
                if (ma_engine_init(NULL, &engine) != MA_SUCCESS)
                    throw std::runtime_error("Failed to initialize audio engine");
            }

            ~AudioEngine() {
                ma_engine_uninit(&engine);
            }

            ma_engine* get() { return &engine; }
        };

        class Sound {
        private:
            ma_sound sound{};
            ma_engine* engine{};
            bool loaded = false;

        public:
            Sound(ma_engine* eng, const std::string& file)
                : engine(eng)
            {
                if (ma_sound_init_from_file(
                    engine,
                    file.c_str(),
                    0,
                    NULL,
                    NULL,
                    &sound) != MA_SUCCESS)
                {
                    throw std::runtime_error("Failed to load sound");
                }

                loaded = true;
            }

            ~Sound() {
                if (loaded)
                    ma_sound_uninit(&sound);
            }

            void Play() {
                ma_sound_start(&sound);
            }

            void Stop() {
                ma_sound_stop(&sound);
            }

            void SetVolume(float v) {
                ma_sound_set_volume(&sound, v);
            }

            void SetLooping(bool loop) {
                ma_sound_set_looping(&sound, loop);
            }

            bool IsPlaying() {
                return ma_sound_is_playing(&sound);
            }
        };

		AudioPlayer::AudioPlayer(const std::string& soundName, ma_engine* engine) {
			sound = std::make_unique<Fox::Audio::Sound>(engine, soundName);
		}

		AudioPlayer::~AudioPlayer() {
			sound = nullptr;
		}

		void AudioPlayer::Play() {
			sound->Play();
		}

		void AudioPlayer::Stop() {
			sound->Stop();
		}

		void AudioPlayer::SetVolume(float volume) {
			sound->SetVolume(volume);
		}

		AudioManager::AudioManager() {
		}

		AudioManager::~AudioManager() {
		}

		void AudioManager::Initialize() {
			audioEngine = std::make_unique<Fox::Audio::AudioEngine>();

			sounds["retro"] = std::make_unique<Fox::Audio::AudioPlayer>("Music/Retro Beating.mp3", audioEngine->get());
		}

		void AudioManager::Destroy() {
			sounds.clear();
			audioEngine = nullptr;
		}
			
		void AudioManager::PlayAudio(const std::string& filePath) {
			sounds[filePath]->SetVolume(1.0f);
			sounds[filePath]->Play();
		}
			
		void AudioManager::StopAudio(const std::string& soundName) {
			sounds[soundName]->Stop();
		}
			
		void AudioManager::SetVolume(const std::string& soundName, float volume) {
			sounds[soundName]->SetVolume(volume);
		}
	}
}