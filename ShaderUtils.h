#pragma once

#include <string>
#include <regex>
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct Variables {
	std::unordered_map<std::string, std::string> uniformMap;
	std::unordered_map<std::string, std::string> inOutMap;
};

Variables extractUniformsAndInOut(
	const std::string& shaderCode, 
	std::unordered_map<std::string, std::string>& globalUniformMap,
	std::unordered_map<std::string, std::string>& globalInOutMap
);

std::string renameVariables(const std::string& shaderCode, const std::unordered_map<std::string, std::string>& variableMap);
