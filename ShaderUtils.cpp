#include "ShaderUtils.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <stack>

// Extracts uniforms and in/out variables from shader code
Variables extractUniformsAndInOut(
	const std::string& shaderCode, 
	std::unordered_map<std::string, std::string>& globalUniformMap,
	std::unordered_map<std::string, std::string>& globalInOutMap
) {
	std::unordered_map<std::string, std::string> uniformMap;
	std::unordered_map<std::string, std::string> inOutMap;

	// Regular expressions to match uniform and in/out variable and array declarations
	std::regex uniformRegex(R"(\buniform\s+\w+\s+((?:\w+(?:\[\d*\])?\s*,\s*)*\w+(?:\[\d*\])?);)");
	std::regex inoutRegex(R"(\b(in|out)\s+\w+\s+((?:\w+(?:\[\d*\])?\s*,\s*)*\w+(?:\[\d*\])?);)");
	std::smatch match;

	auto processVariables = [](const std::string& variables, auto& globalMap, auto& localMap, const std::string& prefix) {
		std::stringstream ss(variables);
		std::string variable;
		while (getline(ss, variable, ',')) {
			variable.erase(std::remove_if(variable.begin(), variable.end(), ::isspace), variable.end()); // remove spaces
			std::string nameOnly = std::regex_replace(variable, std::regex(R"(\[\d*\])"), ""); // remove [N]
			if (globalMap.find(nameOnly) == globalMap.end()) {
				std::string newName = prefix + std::to_string(globalMap.size());
				globalMap[nameOnly] = newName;
			}
			localMap[nameOnly] = globalMap[nameOnly];
		}
	};

	// Extract uniforms
	std::string::const_iterator searchStart(shaderCode.cbegin());
	while (regex_search(searchStart, shaderCode.cend(), match, uniformRegex)) {
		processVariables(match[1], globalUniformMap, uniformMap, "u");
		searchStart = match.suffix().first;
	}

	// Extract in/out variables
	searchStart = shaderCode.cbegin();
	while (regex_search(searchStart, shaderCode.cend(), match, inoutRegex)) {
		processVariables(match[2], globalInOutMap, inOutMap, "a");
		searchStart = match.suffix().first;
	}

	return {uniformMap, inOutMap};
}

// Escapes special characters in a string for regex matching
static std::string escapeRegex(const std::string& str) {
	static const std::regex specialChars(R"([-[\]{}()*+?.,\\^$|#\s])");
	return std::regex_replace(str, specialChars, R"(\$&)");
}

// Finds the scopes of variable redefinitions in shader code
static std::vector<std::pair<size_t, size_t>> findRedefinitionScopes(const std::string& code, const std::string& var) {
	std::vector<std::pair<size_t, size_t>> scopes;
	std::stack<size_t> braceStack;

	for (size_t i = 0; i < code.size(); ++i) {
		if (code[i] == '{') {
			braceStack.push(i);
		}
		else if (code[i] == '}') {
			if (!braceStack.empty()) {
				size_t start = braceStack.top();
				braceStack.pop();
				size_t end = i + 1;
				std::string block = code.substr(start, end - start);

				std::regex defRegex("\\b\\w+\\s+" + var + R"((\s*[\[=;]))");
				if (std::regex_search(block, defRegex)) {
					scopes.emplace_back(start, end);
				}
			}
		}
	}
	return scopes;
}

// Renames variables in shader code based on a mapping
std::string renameVariables(const std::string& shaderCode, const std::unordered_map<std::string, std::string>& variableMap) {
	std::string result = shaderCode;

	for (const auto& [original, replacement] : variableMap) {
		std::string escaped = escapeRegex(original);
		std::regex varRegex("(^|[^\\.\\w])(" + escaped + R"((\b|\[\d*\])))");

		auto scopes = findRedefinitionScopes(result, original);

		std::string updated;
		size_t last = 0;

		for (const auto& [start, end] : scopes) {
			// Replace in the segment before the current scope
			if (start > last) {
				updated += std::regex_replace(result.substr(last, start - last), varRegex, "$1" + replacement + "$3");
			}

			// Copy the scope as-is
			updated.append(result, start, end - start);
			last = end;
		}

		// Replace in the final segment after all scopes
		if (last < result.size()) {
			updated += std::regex_replace(result.substr(last), varRegex, "$1" + replacement + "$3");
		}

		result = std::move(updated);
	}

	return result;
}
