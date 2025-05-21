#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief Reads the content of a file into a string.
 * 
 * @param filePath Path to the file to be read.
 * @return The content of the file as a string.
 * 
 * @throws std::runtime_error if the file cannot be opened or contains non-ASCII characters.
 */
std::string readFile(const std::string& filePath);

/**
 * @brief Writes a string to a file.
 * 
 * @param filePath Path to the file to be written.
 * @param content String to be written to the file.
 * 
 * @throws std::runtime_error if the file cannot be opened for writing.
 */
void writeFile(const std::string& filePath, const std::string& content);

/**
 * @brief Writes binary content to a file.
 * 
 * @param filePath Path to the file to be written.
 * @param content Vector of bytes to be written to the file.
 * 
 * @throws std::runtime_error if the file cannot be opened for writing.
 */
void writeFile(const std::string& filePath, const std::vector<uint8_t>& content);
