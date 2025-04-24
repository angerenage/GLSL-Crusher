#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

struct TokenInfo {
	std::string token;
	int score;
};

struct Tokens {
	std::unordered_map<uint8_t, std::string> token_char_map;
	std::unordered_map<uint16_t, std::string> token_list;
};

TokenInfo find_best_token(
	const std::unordered_map<std::string, std::string>& texts,
	bool find_large,
	size_t minTokenSize,
	size_t maxTokenSize,
	bool verbose
);

Tokens compress_texts(
	std::unordered_map<std::string, std::string>& texts,
	size_t minTokenSize,
	size_t maxTokenSize,
	bool verbose
);
