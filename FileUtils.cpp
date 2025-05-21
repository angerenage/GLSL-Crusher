#include "FileUtils.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>

std::string readFile(const std::string& filePath) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + filePath);
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();

	for (size_t i = 0; i < content.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(content[i]);
		if (c > 127) {
			throw std::runtime_error("Non-ASCII character detected at byte " + std::to_string(i) + " in file " + filePath);
		}
	}

	return content;
}

void writeFile(const std::string& filePath, const std::string& content) {
	std::ofstream file(filePath);
	if (!file) {
		throw std::runtime_error("Failed to open file for writing: " + filePath);
	}
	file << content;
}

void writeFile(const std::string& filePath, const std::vector<uint8_t>& content) {
	std::ofstream file(filePath, std::ios::binary);
	if (!file) {
		throw std::runtime_error("Failed to open file for writing: " + filePath);
	}
	file.write(reinterpret_cast<const char*>(content.data()), content.size());
}
