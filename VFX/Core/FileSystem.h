#pragma once

namespace Fox {

	namespace Core {

		class FileSystem {
		public:
			static std::vector<char> ReadBinaryFile(const std::string& fileName);
			static std::string ReadFile(const std::string& fileName);
			static void WritePNGToDisk(const std::string& fileName, int width, int height, const uint8_t* pixels);
		};
	}
}