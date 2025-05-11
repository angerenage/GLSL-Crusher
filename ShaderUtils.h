#pragma once

#include <string>
#include <unordered_map>

std::string extractExternals(
	std::string code,
	std::unordered_map<std::string, std::string>& globalUniformMap,
	std::unordered_map<std::string, std::string>& globalInOutMap,
	bool verbose = false
);

int extractGLSLVersion(const std::string& code);

void removeGLSLVersionDirective(std::string& code);
