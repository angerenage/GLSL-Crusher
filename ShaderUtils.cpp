#include "ShaderUtils.h"

#include <iostream>
#include <vector>
#include <stack>

// Extracts uniforms and in/out variables from shader code
Variables extractExternals(
	const std::string& code,
	std::unordered_map<std::string, std::string>& globalUniformMap,
	std::unordered_map<std::string, std::string>& globalInOutMap,
	bool verbose
) {
	std::unordered_map<std::string, std::string> uniformMap;
	std::unordered_map<std::string, std::string> inOutMap;

	auto isIdentChar = [](char c) {
		return std::isalnum(c) || c == '_';
	};

	size_t i = 0;
	while (i < code.size()) {
		// Skip spaces
		while (i < code.size() && std::isspace(code[i])) i++;

		// Look for "uniform", "in" or "out"
		auto matchKeyword = [&](std::string_view kw) {
			size_t len = kw.length();
			return code.compare(i, len, kw) == 0 && (i + len == code.size() || !isIdentChar(code[i + len]));
		};

		std::string kind;
		if (matchKeyword("uniform")) {
			kind = "uniform";
			i += 7;
		}
		else if (matchKeyword("in")) {
			kind = "in";
			i += 2;
		}
		else if (matchKeyword("out")) {
			kind = "out";
			i += 3;
		}
		else {
			++i;
			continue;
		}

		while (i < code.size() && std::isspace(code[i])) i++; // Skip space
		while (i < code.size() && isIdentChar(code[i])) ++i; // Parse type (e.g., vec3, float...)
		while (i < code.size() && std::isspace(code[i])) i++; // Skip space

		// Parse list of names
		while (i < code.size()) {
			// Parse identifier
			size_t start = i;
			while (i < code.size() && isIdentChar(code[i])) ++i;
			if (start == i) break;
			std::string name = code.substr(start, i - start);

			// Optionally: [number] or []
			if (i < code.size() && code[i] == '[') {
				++i;
				while (i < code.size() && (std::isdigit(code[i]) || code[i] == ' ')) ++i;
				if (i < code.size() && code[i] == ']') ++i;
			}

			if (verbose) {
				std::cout << "Found " << kind << ": " << name << std::endl;
			}

			// Store
			std::unordered_map<std::string, std::string>& globalMap = (kind == "uniform") ? globalUniformMap : globalInOutMap;
			std::unordered_map<std::string, std::string>& localMap = (kind == "uniform") ? uniformMap : inOutMap;
			if (!name.empty() && globalMap.find(name) == globalMap.end()) {
				std::string prefix = (kind == "uniform") ? "u" : "a";
				globalMap[name] = prefix + std::to_string(globalMap.size());
			}
			localMap[name] = globalMap[name];

			// Skip space
			while (i < code.size() && std::isspace(code[i])) ++i;

			if (i < code.size() && code[i] == ';') {
				++i;
				break;
			}
			else if (i < code.size() && code[i] == ',') {
				++i;
				while (i < code.size() && std::isspace(code[i])) ++i;
			}
			else {
				break;
			}
		}
	}
	return {uniformMap, inOutMap};
}

// Finds the scopes of variable redefinitions in shader code
static std::vector<std::pair<size_t, size_t>> findRedefinitionScopes(const std::string& code, const std::string& var) {
	std::vector<std::pair<size_t, size_t>> scopes;
	std::stack<size_t> braceStack;

	auto isIdentChar = [](char c) {
		return std::isalnum(c) || c == '_';
	};

	auto containsRedef = [&](const std::string& block) -> bool {
		size_t i = 0;

		while (i < block.size()) {
			// Skip whitespace
			while (i < block.size() && std::isspace(block[i])) i++;

			// Skip type
			while (i < block.size() && isIdentChar(block[i])) i++;
			while (i < block.size() && std::isspace(block[i])) i++;

			// Parse variable name
			size_t nameStart = i;
			while (i < block.size() && isIdentChar(block[i])) i++;
			if (nameStart == i) continue;

			std::string found = block.substr(nameStart, i - nameStart);
			if (found == var) return true;

			// Skip possible "[...]", "=...", ";", ","
			while (i < block.size() && block[i] != ';' && block[i] != ',') ++i;
			if (i < block.size()) ++i;
		}

		return false;
	};

	auto containsParamRedef = [&](size_t fnStart, size_t fnEnd) -> bool {
		// Assumes: fnStart is '(', fnEnd is ')'
		size_t i = fnStart + 1;
		while (i < fnEnd) {
			// Skip type
			while (i < fnEnd && isIdentChar(code[i])) ++i;
			while (i < fnEnd && std::isspace(code[i])) ++i;

			// Read name
			size_t nameStart = i;
			while (i < fnEnd && isIdentChar(code[i])) ++i;
			std::string found = code.substr(nameStart, i - nameStart);
			if (found == var) return true;

			// Skip "[...]" and comma
			while (i < fnEnd && code[i] != ',') ++i;
			if (i < fnEnd) ++i;
		}
		return false;
	};

	for (size_t i = 0; i < code.size(); ++i) {
		if (code[i] == '(') {
			// Look for function parameters
			size_t start = i;
			int depth = 1;
			while (i + 1 < code.size() && depth > 0) {
				++i;
				if (code[i] == '(') depth++;
				else if (code[i] == ')') depth--;
			}
			size_t end = i;
			if (containsParamRedef(start, end)) {
				// Look forward for opening brace
				while (end < code.size() && code[end] != '{') ++end;
				if (end < code.size()) {
					braceStack.push(end);
				}
			}
		}
		else if (code[i] == '{') {
			braceStack.push(i);
		}
		else if (code[i] == '}') {
			if (!braceStack.empty()) {
				size_t start = braceStack.top();
				braceStack.pop();
				size_t end = i + 1;
				std::string block = code.substr(start, end - start);
				if (containsRedef(block)) {
					scopes.emplace_back(start, end);
				}
			}
		}
	}
	return scopes;
}

// Renames variables in shader code based on a mapping
std::string renameVariables(
	const std::string& code,
	const std::unordered_map<std::string, std::string>& variableMap
) {
	std::string result;
	size_t i = 0;

	while (i < code.size()) {
		bool matched = false;

		for (const auto& [name, replacement] : variableMap) {
			if (code.compare(i, name.size(), name) == 0) {
				// Ensure it's a word boundary
				bool left = (i == 0) || !std::isalnum(code[i - 1]);
				bool right = (i + name.size() >= code.size()) || !std::isalnum(code[i + name.size()]);
				if (left && right) {
					result += replacement;
					i += name.size();
					matched = true;
					break;
				}
			}
		}

		if (!matched) {
			result += code[i++];
		}
	}

	return result;
}

// Retrieves the GLSL version directive position from shader code
int extractGLSLVersion(const std::string& code) {
	size_t pos = code.find("#version");
	if (pos == std::string::npos) return 0;

	pos += 8;
	while (pos < code.size() && std::isspace(code[pos])) ++pos;

	size_t start = pos;
	while (pos < code.size() && std::isdigit(code[pos])) ++pos;

	if (start == pos) return 0;
	return std::stoi(code.substr(start, pos - start));
}

// Removes the GLSL version directive from shader code
void removeGLSLVersionDirective(std::string& code) {
	size_t pos = code.find("#version");
	if (pos == std::string::npos) return;

	size_t end = pos;
	while (end < code.size() && code[end] != '\n') ++end;
	if (end < code.size()) ++end; // include newline

	code.erase(pos, end - pos);
}
