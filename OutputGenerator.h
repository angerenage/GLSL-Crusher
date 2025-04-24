#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

std::string generateHeader(
	const std::unordered_map<std::string, std::string>& variableMap,
	const std::vector<std::pair<std::string, size_t>>& shaderOffsets
);

std::string generateCFile(
	const std::unordered_map<uint8_t, std::string>& tokenCharMap,
	const std::unordered_map<std::string, std::string>& variableMap,
	const std::string& glslVersion,
	size_t maxOutputSize
);

std::vector<uint8_t> generatePackedContent(
	const std::unordered_map<std::string, std::string>& shaders,
	const std::unordered_map<uint16_t, std::string>& tokenList,
	std::vector<std::pair<std::string, size_t>>& shadersOffsets
);
