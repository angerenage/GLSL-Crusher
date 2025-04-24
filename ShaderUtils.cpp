#include "ShaderUtils.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

// Extracts uniforms and in/out variables from shader code
Variables extractUniformsAndInOut(
	const std::string& shaderCode, 
	std::unordered_map<std::string, std::string>& globalUniformMap,
	std::unordered_map<std::string, std::string>& globalInOutMap
) {
	std::unordered_map<std::string, std::string> uniformMap;
	std::unordered_map<std::string, std::string> inOutMap;

	std::regex uniformRegex(R"(\buniform\s+\w+\s+(\w+(?:,\w+)*);)");
	std::regex inoutRegex(R"(\b(in|out)\s+\w+\s+(\w+(?:,\w+)*);)");
	std::smatch match;

	// Extract uniforms
	std::string::const_iterator searchStart(shaderCode.cbegin());
	while (regex_search(searchStart, shaderCode.cend(), match, uniformRegex)) {
		std::string variables = match[1];
		std::stringstream ss(variables);
		std::string variable;
		while (getline(ss, variable, ',')) {
			if (globalUniformMap.find(variable) == globalUniformMap.end()) {
				std::string newName = "u" + std::to_string(globalUniformMap.size());
				globalUniformMap[variable] = newName;
			}
			uniformMap[variable] = globalUniformMap[variable];
		}
		searchStart = match.suffix().first;
	}

	// Extract in/out variables
	searchStart = shaderCode.cbegin();
	while (regex_search(searchStart, shaderCode.cend(), match, inoutRegex)) {
		std::string variables = match[2];
		std::stringstream ss(variables);
		std::string variable;
		while (getline(ss, variable, ',')) {
			if (globalInOutMap.find(variable) == globalInOutMap.end()) {
				std::string newName = "a" + std::to_string(globalInOutMap.size());
				globalInOutMap[variable] = newName;
			}
			inOutMap[variable] = globalInOutMap[variable];
		}
		searchStart = match.suffix().first;
	}

	return {uniformMap, inOutMap};
}

// Renames variables in shader code based on a mapping
std::string renameVariables(const std::string& shaderCode, const std::unordered_map<std::string, std::string>& variableMap) {
	std::string minifiedCode = shaderCode;
	for (const auto& [original, minified] : variableMap) {
		std::regex varRegex("\\b" + original + "\\b");
		minifiedCode = std::regex_replace(minifiedCode, varRegex, minified);
	}
	return minifiedCode;
}
