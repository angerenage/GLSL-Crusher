#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Generates a C header file containing the shader offsets and function prototypes.
 * 
 * @param variableMap Map of variable names to their new names.
 * @param shaderOffsets Vector of pairs containing shader names and their offsets in the packed content.
 * @return The generated header content as a string.
 */
std::string generateHeader(
	const std::unordered_map<std::string, std::string>& variableMap,
	const std::vector<std::pair<std::string, size_t>>& shaderOffsets
);

/**
 * @brief Generates a C file containing the uniforms new names and unpacking functions for the packed shaders.
 * 
 * @param tokenCharMap Lookup table for the compressed tokens.
 * @param variableMap Map of variable names to their new names.
 * @param glslVersion Version directive for the unpaked GLSL code.
 * @param maxOutputSize Maximum size of the output buffer used in the generated code.
 * @return The generated C file content as a string.
 */
std::string generateCFile(
	const std::unordered_map<uint8_t, std::string>& tokenCharMap,
	const std::unordered_map<std::string, std::string>& variableMap,
	const std::string& glslVersion,
	size_t maxOutputSize
);

/**
 * @brief Generates packed content for the shaders and their offsets.
 * 
 * @param shaders Map of shader names to their GLSL code.
 * @param tokenList Map of token offsets to their string representations.
 * @param shadersOffsets Vector of pairs containing shader names and their offsets in the packed content.
 * @return The packed content as a vector of bytes.
 * 
 * @throws std::runtime_error if the token list is malformed.
 */
std::vector<uint8_t> generatePackedContent(
	const std::unordered_map<std::string, std::string>& shaders,
	const std::unordered_map<uint16_t, std::string>& tokenList,
	std::vector<std::pair<std::string, size_t>>& shadersOffsets
);
