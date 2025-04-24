#pragma once

#include <cstdint>
#include <string>
#include <vector>

std::string readFile(const std::string& filePath);
void writeFile(const std::string& filePath, const std::string& content);
void writeFile(const std::string& filePath, const std::vector<uint8_t>& content);
