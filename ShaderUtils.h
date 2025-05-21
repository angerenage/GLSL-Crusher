#pragma once

#include <string>
#include <unordered_map>

/**
 * @brief Extracts and renames external variables, uniforms, ins and outs, from GLSL code.
 *
 * @param code The GLSL shader code as a string.
 * @param globalUniformMap Map to store extracted uniform variable names and their new name.
 * @param globalInOutMap Map to store extracted in/out variable names and their new name.
 * @param verbose If true, enables verbose output for debugging.
 * @return The GLSL code with externals extracted.
 */
std::string extractExternals(
	std::string code,
	std::unordered_map<std::string, std::string>& globalUniformMap,
	std::unordered_map<std::string, std::string>& globalInOutMap,
	bool verbose
);

/**
 * @brief Extracts the GLSL version directive from shader code.
 *
 * @param code The GLSL shader code as a string.
 * @return The GLSL version number, or 0 if not found.
 */
int extractGLSLVersion(const std::string& code);

/**
 * @brief Removes the GLSL version directive from the shader code string in-place.
 *
 * @param code The GLSL shader code as a string (modified in-place).
 */
void removeGLSLVersionDirective(std::string& code);
