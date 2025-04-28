#include "Token.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <limits>

// Function to find the best token based on occurrences and scoring
TokenInfo find_best_token(const std::unordered_map<std::string, std::string>& texts, bool find_large, size_t minTokenSize, size_t maxTokenSize, bool verbose) {
	std::unordered_map<std::string, int> occurrences;

	for (const auto& [name, text] : texts) {
		size_t text_length = text.size();

		// Limit the length of the text to maxTokenSize
		if (maxTokenSize > 0 && text_length > maxTokenSize) text_length = maxTokenSize;

		// Count occurrences of substrings
		if (text_length >= minTokenSize) {
			for (size_t len = minTokenSize; len <= text_length; len++) {
				for (int i = 0; i <= text_length - len; i++) {
					// Cutting just after a '$'
					if ((i > 0 && text[i - 1] == '$') || (i > 1 && text[i - 2] == '$')) continue;

					// Token ends with '$' followed by only one char (partial $N)
					if (i + len < text_length && (text[i + len - 1] == '$' || text[i + len - 2] == '$')) continue;

					std::string sub = text.substr(i, len);

					occurrences[sub]++;
				}
			}
		}
	}

	TokenInfo best_token = {"", -1};
	// Calculate scores
	for (const auto& [token, count] : occurrences) {
		// Only interesting if the string appears more than once
		if (count > 1) {
			int token_length = (int)token.size();
			int long_rep = find_large ? 3 : 1;
			int score = token_length * (count - 1) - count * long_rep - 1;

			// Keep only the best token
			if (score > best_token.score) {
				best_token = { token, score };
			}
		}
	}

	return best_token;
}

// Function to replace tokens in the text with a replacement string
void replace_tokens(std::unordered_map<std::string, std::string>& texts, const std::string& token, const std::string& replacement) {
	for (auto& [name, text] : texts) {
		std::ostringstream oss;
		std::size_t pos = 0, last_pos = 0;

		while ((pos = text.find(token, pos)) != std::string::npos) {
			oss.write(text.data() + last_pos, pos - last_pos);
			oss << replacement;
			pos += token.size();
			last_pos = pos;
		}
		oss.write(text.data() + last_pos, text.size() - last_pos);
		text = oss.str();
	}
}

// Function to compress texts by finding and replacing tokens
Tokens compress_texts(std::unordered_map<std::string, std::string>& texts, size_t minTokenSize, size_t maxTokenSize, bool verbose) {
	std::unordered_map<uint8_t, std::string> token_char_map;
	std::unordered_map<uint16_t, std::string> token_list;

	if (verbose) {
		std::cout << "Compressing texts with minTokenSize: " << minTokenSize;
		if (maxTokenSize > 0) {
			std::cout << ", maxTokenSize: " << maxTokenSize;
		}
		std::cout << std::endl;
	}

	// Find and replace single-character tokens
	for (int token_value = 128; token_value <= 255; token_value++) {
		TokenInfo best_token = find_best_token(texts, false, minTokenSize, maxTokenSize, verbose);
		if (best_token.score <= 0 || best_token.token.empty()) {
			break;
		}

		if (verbose) {
			std::cout << "Best token found (" << (token_value - 128 + 1) << "/128), of length " << best_token.token.length() << " and with score: " << best_token.score << std::endl;
		}

		token_char_map[token_value] = best_token.token;
		std::string replacement = std::string(1, static_cast<uint8_t>(token_value));
		replace_tokens(texts, best_token.token, replacement);
	}

	if (verbose) {
		std::cout << "Found " << token_char_map.size() << " single-character tokens." << std::endl;
	}

	size_t offset = 0;
	// Find and replace multi-character tokens
	while (offset < std::numeric_limits<uint16_t>::max()) {
		TokenInfo best_token = find_best_token(texts, true, minTokenSize, maxTokenSize, verbose);
		if (best_token.score <= 0 || best_token.token.empty()) {
			break;
		}

		if (verbose) {
			std::cout << "Best token found (" << offset << "), of length " << best_token.token.length() << " and with score: " << best_token.score << std::endl;
		}

		token_list.insert({(uint16_t)offset, best_token.token});

		std::string replacement = "$";
		replacement += static_cast<char>(offset & 0xFF);
		replacement += static_cast<char>((offset >> 8) & 0xFF);
		replace_tokens(texts, best_token.token, replacement);

		offset += best_token.token.size() + 1;
	}

	if (verbose && !token_list.empty()) {
		std::cout << "Found " << token_list.size() << " multi-character tokens." << std::endl;
	}

	return {token_char_map, token_list};
}
