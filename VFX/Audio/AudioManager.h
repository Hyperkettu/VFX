#pragma once 

#include <string>
#include <unordered_map>

#include <string>
#include <stdexcept>

#include "VFX/Core/Singleton.h"

class ma_engine;

namespace Fox {

	namespace Audio {

		class Sound;
		class AudioEngine;

		class AudioPlayer {
		public:
			AudioPlayer() = default;
			AudioPlayer(const std::string& soundName, ma_engine* engine);
			~AudioPlayer();


			void Play();
			void Stop();
			void SetVolume(float volume);

		private:
            std::unique_ptr<Fox::Audio::Sound> sound = nullptr;
		};

		class AudioManager: public Fox::Core::Singleton<Fox::Audio::AudioManager> {
		public:
			AudioManager();
			~AudioManager();
            
			void Initialize();

			void PlayAudio(const std::string& filePath);
			void StopAudio(const std::string& soundName);
			void SetVolume(const std::string& soundName, float volume);

			void Destroy();

		private: 
			std::unique_ptr<Fox::Audio::AudioEngine> audioEngine;
			std::unordered_map<std::string, std::unique_ptr<Fox::Audio::AudioPlayer>> sounds;
		};
	}
}