#include "FileUtils.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>

// Function to read content from a file to a string
std::string readFile(const std::string& filePath) {
	std::ifstream file(filePath);
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

// Function to write a string to a file
void writeFile(const std::string& filePath, const std::string& content) {
	std::ofstream file(filePath);
	if (!file) {
		throw std::runtime_error("Failed to open file for writing: " + filePath);
	}
	file << content;
}

// Function to write binary content to a file
void writeFile(const std::string& filePath, const std::vector<uint8_t>& content) {
	std::ofstream file(filePath, std::ios::binary);
	if (!file) {
		throw std::runtime_error("Failed to open file for writing: " + filePath);
	}
	file.write(reinterpret_cast<const char*>(content.data()), content.size());
}
