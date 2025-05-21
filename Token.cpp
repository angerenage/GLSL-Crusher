#include "Token.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <limits>

/**
 * @struct TokenInfo
 * @brief Holds information about a token, including its string representation and score.
 */
struct TokenInfo {
	std::string token; ///< The token string.
	int score;         ///< The score of the token based on its occurrences and length.
};

/**
 * @brief Finds the best substring to use as a token in a set of texts, based on occurrences and length.
 *
 * @param texts Map of text identifiers to their string contents.
 * @param findLarge If true, uses 2-byte offset tokens; if false, uses 1-byte tokens.
 * @param minTokenSize Minimum length of substrings to consider as tokens.
 * @param maxTokenSize Maximum length of substrings to consider as tokens.
 * @param verbose If true, enables verbose output for debugging and analysis.
 * @return A TokenInfo struct containing the best token and its score.
 */
static TokenInfo findBestToken(const std::unordered_map<std::string, std::string>& texts, bool findLarge, size_t minTokenSize, size_t maxTokenSize, bool verbose) {
	std::unordered_map<std::string, int> occurrences;

	for (const auto& [name, text] : texts) {
		size_t textLength = text.size();

		// Limit the length of the text to maxTokenSize
		if (maxTokenSize > 0 && textLength > maxTokenSize) textLength = maxTokenSize;

		// Count occurrences of substrings
		if (textLength >= minTokenSize) {
			for (size_t len = minTokenSize; len <= textLength; len++) {
				for (int i = 0; i <= textLength - len; i++) {
					// Cutting just after a '$'
					if ((i > 0 && text[i - 1] == '$') || (i > 1 && text[i - 2] == '$')) continue;

					// Token ends with '$' followed by only one char (partial $N)
					if (i + len < textLength && (text[i + len - 1] == '$' || text[i + len - 2] == '$')) continue;

					std::string sub = text.substr(i, len);

					occurrences[sub]++;
				}
			}
		}
	}

	TokenInfo bestToken = {"", -1};
	// Calculate scores
	for (const auto& [token, count] : occurrences) {
		// Only interesting if the string appears more than once
		if (count > 1) {
			int tokenLength = (int)token.size();
			int longRep = findLarge ? 3 : 1;
			int score = tokenLength * (count - 1) - count * longRep - 1;

			// Keep only the best token
			if (score > bestToken.score) {
				bestToken = { token, score };
			}
		}
	}

	return bestToken;
}

/**
 * @brief Replaces occurrences of a token in the texts with a replacement string.
 * 
 * @param texts Map of text in which to replace tokens, where each text is identified by a its file path.
 * @param token The token to replace.
 * @param replacement The string to replace the token with.
 */
static void replaceTokens(std::unordered_map<std::string, std::string>& texts, const std::string& token, const std::string& replacement) {
	for (auto& [name, text] : texts) {
		std::ostringstream oss;
		std::size_t pos = 0, lastPos = 0;

		while ((pos = text.find(token, pos)) != std::string::npos) {
			oss.write(text.data() + lastPos, pos - lastPos);
			oss << replacement;
			pos += token.size();
			lastPos = pos;
		}
		oss.write(text.data() + lastPos, text.size() - lastPos);
		text = oss.str();
	}
}

Tokens compressTexts(std::unordered_map<std::string, std::string>& texts, size_t minTokenSize, size_t maxTokenSize, bool verbose) {
	std::unordered_map<uint8_t, std::string> tokenCharMap;
	std::unordered_map<uint16_t, std::string> tokenOffsetMap;

	if (verbose) {
		std::cout << "Compressing texts with minTokenSize: " << minTokenSize;
		if (maxTokenSize > 0) {
			std::cout << ", maxTokenSize: " << maxTokenSize;
		}
		std::cout << std::endl;
	}

	// Find and replace single-character tokens
	for (int tokenValue = 128; tokenValue <= 255; tokenValue++) {
		TokenInfo bestToken = findBestToken(texts, false, minTokenSize, maxTokenSize, verbose);
		if (bestToken.score <= 0 || bestToken.token.empty()) {
			break;
		}

		if (verbose) {
			std::cout << "Best token found (" << (tokenValue - 128 + 1) << "/128), of length " << bestToken.token.length() << " and with score: " << bestToken.score << std::endl;
		}

		tokenCharMap[tokenValue] = bestToken.token;
		std::string replacement = std::string(1, static_cast<uint8_t>(tokenValue));
		replaceTokens(texts, bestToken.token, replacement);
	}

	if (verbose) {
		std::cout << "Found " << tokenCharMap.size() << " single-character tokens." << std::endl;
	}

	size_t offset = 0;
	// Find and replace multi-character tokens
	while (offset < std::numeric_limits<uint16_t>::max()) {
		TokenInfo bestToken = findBestToken(texts, true, minTokenSize, maxTokenSize, verbose);
		if (bestToken.score <= 0 || bestToken.token.empty()) {
			break;
		}

		if (verbose) {
			std::cout << "Best token found (" << offset << "), of length " << bestToken.token.length() << " and with score: " << bestToken.score << std::endl;
		}

		tokenOffsetMap.insert({(uint16_t)offset, bestToken.token});

		std::string replacement = "$";
		replacement += static_cast<char>(offset & 0xFF);
		replacement += static_cast<char>((offset >> 8) & 0xFF);
		replaceTokens(texts, bestToken.token, replacement);

		offset += bestToken.token.size() + 1;
	}

	if (verbose && !tokenOffsetMap.empty()) {
		std::cout << "Found " << tokenOffsetMap.size() << " multi-character tokens." << std::endl;
	}

	return {tokenCharMap, tokenOffsetMap};
}
