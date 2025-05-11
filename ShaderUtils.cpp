#include "ShaderUtils.h"

#include <iostream>
#include <vector>
#include <unordered_set>
#include <stack>

// Extracts and renames external variables, uniforms, ins and outs, in GLSL code
std::string extractExternals(
	std::string code,
	std::unordered_map<std::string, std::string>& globalUniformMap,
	std::unordered_map<std::string, std::string>& globalInOutMap,
	bool verbose
) {
	auto isIdentChar = [](char c) {
		return std::isalnum(c) || c == '_';
	};

	std::vector<std::unordered_set<std::string>> scopeStack;

	auto isShadowed = [&](const std::string& name) {
		for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); it++) {
			if (it->count(name)) return true;
		}
		return false;
	};

	std::unordered_map<std::string, std::string> localUniformMap;
	std::unordered_map<std::string, std::string> localInOutMap;

	std::string output;
	std::string currentToken;
	std::string lastIdent;
	std::string declKind;

	enum class DeclState { None, FoundDecl, ScanningVar };
	DeclState declState = DeclState::None;

	bool insideFunctionArgs = false;
	int parenDepth = 0;

	size_t i = 0;
	while (i < code.size()) {
		char c = code[i];

		// Handle function arguments
		if (c == '(') {
			parenDepth++;
			if (!output.empty() && isIdentChar(output.back())) {
				insideFunctionArgs = true;
			}
			output += c;
			i++;
			continue;
		}
		if (c == ')') {
			if (parenDepth > 0) parenDepth--;
			if (parenDepth == 0) insideFunctionArgs = false;
			output += c;
			i++;
			continue;
		}

		// Handle scopes
		if (c == '{') {
			scopeStack.emplace_back();
			output += c;
			i++;
			continue;
		}
		if (c == '}') {
			if (!scopeStack.empty()) scopeStack.pop_back();
			output += c;
			i++;
			continue;
		}

		// Identifiers
		if (isIdentChar(c)) {
			currentToken.clear();
			while (i < code.size() && isIdentChar(code[i])) currentToken += code[i++];

			// Start of a declaration
			if (!insideFunctionArgs && declState == DeclState::None && (currentToken == "uniform" || currentToken == "in" || currentToken == "out")) {
				declKind = currentToken;
				declState = DeclState::FoundDecl;
				output += currentToken;
				continue;
			}

			// Token after 'uniform', 'in', or 'out'
			if (declState == DeclState::FoundDecl || declState == DeclState::ScanningVar) {
				lastIdent = currentToken;
				declState = DeclState::ScanningVar;
				output += currentToken;
				continue;
			}

			// Regular token: replace if global and not shadowed
			if (!isShadowed(currentToken)) {
				auto it = globalUniformMap.find(currentToken);
				if (it != globalUniformMap.end()) {
					output += it->second;
					continue;
				}
				it = globalInOutMap.find(currentToken);
				if (it != globalInOutMap.end()) {
					output += it->second;
					continue;
				}
			}
			output += currentToken;
			continue;
		}

		// End of declaration or list of variables
		if (c == ';' || c == ',' || c == '[' || c == '=') {
			if (declState == DeclState::ScanningVar && !lastIdent.empty()) {
				std::string replacement;
				if (declKind == "uniform") {
					auto it = globalUniformMap.find(lastIdent);
					if (it != globalUniformMap.end()) {
						replacement = it->second;
					}
					else {
						replacement = "u" + std::to_string(globalUniformMap.size());
						globalUniformMap[lastIdent] = replacement;
					}
				}
				else {
					auto it = globalInOutMap.find(lastIdent);
					if (it != globalInOutMap.end()) {
						replacement = it->second;
					}
					else {
						replacement = "a" + std::to_string(globalInOutMap.size());
						globalInOutMap[lastIdent] = replacement;
					}
				}
				if (verbose) std::cout << "Found " << declKind << ": " << lastIdent << " -> " << replacement << "\n";

				if (!scopeStack.empty()) scopeStack.back().insert(lastIdent);

				// Replace last identifier in output
				size_t pos = output.size();
				while (pos > 0 && isIdentChar(output[pos - 1])) pos--;
				output.erase(pos);
				output += replacement;

				lastIdent.clear();
			}

			if (c == ';') declState = DeclState::None;
		}

		// Copy anything else
		output += c;
		i++;
	}

	return output;
}

// Retrieves the GLSL version directive position from shader code
int extractGLSLVersion(const std::string& code) {
	size_t pos = code.find("#version");
	if (pos == std::string::npos) return 0;

	pos += 8;
	while (pos < code.size() && std::isspace(code[pos])) pos++;

	size_t start = pos;
	while (pos < code.size() && std::isdigit(code[pos])) pos++;

	if (start == pos) return 0;
	return std::stoi(code.substr(start, pos - start));
}

// Removes the GLSL version directive from shader code
void removeGLSLVersionDirective(std::string& code) {
	size_t pos = code.find("#version");
	if (pos == std::string::npos) return;

	size_t end = pos;
	while (end < code.size() && code[end] != '\n') end++;
	if (end < code.size()) end++; // include newline

	code.erase(pos, end - pos);
}
