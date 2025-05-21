#include "ShaderUtils.h"

#include <iostream>
#include <vector>
#include <unordered_set>
#include <stack>

std::string extractExternals(
	std::string code,
	std::unordered_map<std::string, std::string>& globalUniformMap,
	std::unordered_map<std::string, std::string>& globalInOutMap,
	bool verbose
) {
	auto replaceGlobal = [&](const std::string& name) -> std::string {
		if (globalUniformMap.count(name)) return globalUniformMap[name];
		if (globalInOutMap.count(name)) return globalInOutMap[name];
		return {};
	};

	std::vector<std::unordered_set<std::string>> scopeStack;
	std::vector<std::string> params;

	auto isShadowed = [&](const std::string& name) -> bool {
		for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it)
			if (it->count(name)) return true;
		for (const auto& p : params)
			if (p == name) return true;
		return false;
	};

	std::string output;
	std::string currentToken, declKind;

	int parenDepth = 0;
	int previousWords = 0;

	char previousNonSpace = '\0';

	size_t i = 0;
	
	auto nextNonSpace = [&](char c) -> char {
		for (size_t j = i; j < code.size(); j++) {
			if (!std::isspace(static_cast<unsigned char>(code[j]))) {
				return code[j];
			}
		}
		return c;
	};

	while (i < code.size()) {
		char c = code[i];

		// --- IDENTIFIERS ---
		if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
			currentToken += c;
		}
		else if (std::isdigit(static_cast<unsigned char>(c))) {
			if (!currentToken.empty()) currentToken += c;
			else output += c;
		}
		else {
			// --- PARENTHESES ---
			if (c == '(') {
				params.clear();
				parenDepth++;
			}
			// --- SCOPES ---
			else if (c == '{') {
				scopeStack.emplace_back();
				if (!params.empty()) {
					for (const auto& p : params) scopeStack.back().insert(p);
					params.clear();
				}
			}
			else if (c == '}') {
				if (!scopeStack.empty()) scopeStack.pop_back();
			}
			// --- WHITESPACE AND PUNCTUATION ---
			else if (std::isspace(static_cast<unsigned char>(c)) || std::ispunct(static_cast<unsigned char>(c))) {
				if (i >= 1 && !std::isspace(static_cast<unsigned char>(code[i - 1])) && !currentToken.empty()) {
					previousWords++;
				}
			}

			// --- END OF TOKEN ---
			if (!currentToken.empty()) {
				char nextChar = nextNonSpace(c);

				if (parenDepth == 0 && (currentToken == "uniform" || currentToken == "in" || currentToken == "out")) {
					declKind = currentToken;
				}
				else if (std::ispunct(static_cast<unsigned char>(nextChar))) {
					if (parenDepth > 0 && previousWords > 1) {
						params.push_back(currentToken);
					}
					else if (parenDepth == 0 && !declKind.empty()) {
						auto& map = (declKind == "uniform" ? globalUniformMap : globalInOutMap);
						auto it = map.find(currentToken);
						if (it == map.end()) map[currentToken] = (declKind == "uniform" ? "u" : "a") + std::to_string(map.size());

						if (verbose) std::cout << "Found " << declKind << ": " << currentToken << " -> " << map[currentToken] << "\n";

						currentToken = map[currentToken];
					}
					else if (previousNonSpace != '.' && !isShadowed(currentToken)) {
						if (previousWords > 1) {
							if (scopeStack.empty()) scopeStack.emplace_back();
							scopeStack.back().insert(currentToken);
						}
						else {
							std::string replacement = replaceGlobal(currentToken);
							if (!replacement.empty()) {
								printf("Replacing %s with %s\n", currentToken.c_str(), replacement.c_str());
								currentToken = replacement;
							}
						}
					}

					if (c == ';') {
						declKind.clear();
						if (parenDepth > 0 && !params.empty()) params.clear();
					}
				}

				output += currentToken;
				currentToken.clear();
			}

			if (c == ')') {
				if (parenDepth > 0) parenDepth--;
			}

			if (std::ispunct(static_cast<unsigned char>(c))) previousWords = 0;

			output += c;
			previousNonSpace = c;
		}
		
		i++;
	}

	return output;
}

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

void removeGLSLVersionDirective(std::string& code) {
	size_t pos = code.find("#version");
	if (pos == std::string::npos) return;

	size_t end = pos;
	while (end < code.size() && code[end] != '\n') end++;
	if (end < code.size()) end++; // include newline

	code.erase(pos, end - pos);
}
