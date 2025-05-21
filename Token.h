#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

/**
 * @struct Tokens
 * @brief Holds mappings for compressed token representations.
 */
struct Tokens {
	std::unordered_map<uint8_t, std::string> tokenCharMap; ///< Maps single-byte tokens to strings
	std::unordered_map<uint16_t, std::string> tokenOffsetMap; ///< Maps 16-bit offset tokens to strings
};

/**
 * @brief Compresses a set of texts by finding and replacing repeated substrings with tokens.
 *
 * @param texts Map of text identifiers to their string contents. Modified in-place with compressed results.
 * @param minTokenSize Minimum length of substrings to consider as tokens.
 * @param maxTokenSize Maximum length of substrings to consider as tokens.
 * @param verbose If true, enables verbose output for debugging and analysis.
 * @return A Tokens struct containing the mappings of tokens to their string representations.
 */
Tokens compressTexts(
	std::unordered_map<std::string, std::string>& texts,
	size_t minTokenSize,
	size_t maxTokenSize,
	bool verbose
);
