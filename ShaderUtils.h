#pragma once

#include <string>
#include <unordered_map>

struct Variables {
	std::unordered_map<std::string, std::string> uniformMap;
	std::unordered_map<std::string, std::string> inOutMap;
};

Variables extractExternals(
	const std::string& shaderCode, 
	std::unordered_map<std::string, std::string>& globalUniformMap,
	std::unordered_map<std::string, std::string>& globalInOutMap,
	bool verbose
);

std::string renameVariables(const std::string& shaderCode, const std::unordered_map<std::string, std::string>& variableMap);

int extractGLSLVersion(const std::string& code);

void removeGLSLVersionDirective(std::string& code);
