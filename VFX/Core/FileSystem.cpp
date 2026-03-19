#include "FoxRenderer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <STB_image/stb_image_write.h>

namespace Fox {

	namespace Core {

		std::vector<char> FileSystem::ReadBinaryFile(const std::string& fileName) {
			std::ifstream file(fileName, std::ios::ate | std::ios::binary);

			if (!file.is_open()) {
				throw std::runtime_error("Failed to open file!");
			}

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);
			file.seekg(0);
			file.read(buffer.data(), fileSize);
			file.close();

			return buffer;
		}

		std::string FileSystem::ReadFile(const std::string& fileName) {
			std::ifstream file(fileName);
			std::string line;
			std::stringstream stringStream;

			if (file.is_open()) {
				while (std::getline(file, line)) {
					stringStream << line << "\n";
				}
			} else {
				std::cout << "Could not open file " << fileName << std::endl;
			}

			file.close();

			return stringStream.str();
		}

		void FileSystem::WritePNGToDisk(const std::string& fileName, int width, int height, const uint8_t* pixels) {
			int channels = 4; // Assuming RGBA format
			stbi_write_png(
				fileName.c_str(),
				width,
				height,
				channels,
				pixels,
				width * channels // stride (bytes per row)
			);

		}

	}
}