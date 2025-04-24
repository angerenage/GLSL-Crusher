#include "OutputGenerator.h"
#include "FileUtils.h"

#include <string_view>
#include <sstream>
#include <iomanip>

// Function to decompress shader source from packed content
static constexpr std::string_view getShaderSourceFromFileSrc = R"(static void decompress(const char* compressedText, size_t offset, char* decompressedText, size_t* write_pos) {
	for (size_t readPos = offset; compressedText[readPos] != '\0'; readPos++) {
		unsigned char c = (unsigned char)compressedText[readPos];

		if (c == '$') {
			if (readPos + 2 < MAX_OUTPUT_SIZE) {
				uint16_t tokenOffset = *((uint16_t*)&compressedText[readPos + 1]);
				readPos += 2;

				size_t tokenLength = strlen(&compressedText[tokenOffset]);
				if (*write_pos + tokenLength >= MAX_OUTPUT_SIZE) {
					fprintf(stderr, "Error: Output buffer overflow\n");
					return;
				}

				decompress(&compressedText[tokenOffset], 0, decompressedText, write_pos);
			}
			else {
				fprintf(stderr, "Error: Unexpected end of file\n");
				return;
			}
		}
		else if (c >= 128 && c <= 255) {
			size_t tokenLength = strlen(tokens[c - 128]);
			if (*write_pos + tokenLength >= MAX_OUTPUT_SIZE) {
				fprintf(stderr, "Error: Output buffer overflow\n");
				return;
			}

			decompress(tokens[c - 128], 0, decompressedText, write_pos);
		}
		else {
			decompressedText[(*write_pos)++] = c;
		}
	}
	decompressedText[*write_pos] = '\0';
}

char* getShaderSourceFromFile(const char* compressedText, size_t offset) {
	char* decompressedText = (char*)malloc(MAX_OUTPUT_SIZE + 1);
	if (!decompressedText) {
		fprintf(stderr, "Memory allocation error\n");
		return NULL;
	}
	
	strncpy(decompressedText, version, VERSION_LENGTH);
	decompressedText[VERSION_LENGTH] = '\0';

	size_t write_pos = VERSION_LENGTH;
	decompress(compressedText, offset, decompressedText, &write_pos);
	
	return decompressedText;
})";

// Function to generate header file for shaders
std::string generateHeader(
	const std::unordered_map<std::string, std::string>& variableMap,
	const std::vector<std::pair<std::string, size_t>>& shaderOffsets
) {
	std::ostringstream headerContent;

	headerContent << "#pragma once" << std::endl << std::endl;

	headerContent << "enum ShaderOffset {" << std::endl;
	for (const auto& [name, offset] : shaderOffsets) {
		size_t pos1 = name.find_last_of('/');
		pos1 = (pos1 == std::string::npos) ? 0 : pos1;
		size_t pos2 = name.find_last_of('\\');
		pos2 = (pos2 == std::string::npos) ? 0 : pos2;
		size_t begin = std::max(pos1, pos2) + 1;

		std::string processedName = name.substr(begin, name.find_last_of('.') - begin);
		if (isdigit(processedName[0])) processedName = "_" + processedName;

		headerContent << "\t" << processedName << " = " << std::to_string(offset) << "," << std::endl;
	}
	headerContent << "};" << std::endl << std::endl;

	for (const auto& [original, minified] : variableMap) {
		if (minified[0] == 'u') {
			headerContent << "extern const char* " << original << ";" << std::endl;
		}
	}

	headerContent << std::endl;
	headerContent << "char* getShaderSourceFromFile(const char* compressedText, size_t offset);" << std::endl;

	return headerContent.str();
}

// Function to generate the C file content for shaders
std::string generateCFile(
	const std::unordered_map<uint8_t, std::string>& tokenCharMap,
	const std::unordered_map<std::string, std::string>& variableMap,
	const std::string& glslVersion,
	size_t maxOutputSize
) {
	std::ostringstream cFileContent;

	cFileContent << "#include <stdint.h>" << std::endl;
	cFileContent << "#include <stdlib.h>" << std::endl;
	cFileContent << "#include <string.h>" << std::endl;
	cFileContent << "#include <stdio.h>" << std::endl << std::endl;

	cFileContent << "#define MAX_OUTPUT_SIZE " << std::to_string(maxOutputSize + glslVersion.length() + 2) << std::endl;
	cFileContent << "#define VERSION_LENGTH " << std::to_string(glslVersion.length() + 1) << std::endl << std::endl;

	cFileContent << "const char* version = \"" << glslVersion << "\\n\";" << std::endl << std::endl;

	cFileContent << "const char* tokens[] = {" << std::endl;
	for (uint8_t i = 128; i <= 255; i++) {
		if (tokenCharMap.find(i) != tokenCharMap.end()) {
			cFileContent << "\t\"";
	
			const std::string &str = tokenCharMap.at(i);
			for (size_t j = 0; j < str.size(); ++j) {
				unsigned char c = static_cast<unsigned char>(str[j]);
	
				// If the character is printable and not a special escape character
				if (isprint(c) && c != '\"' && c != '\\') {
					cFileContent << c;
				}
				// Escape double quotes
				else if (c == '\"') {
					cFileContent << "\\\"";
				}
				// Escape backslashes
				else if (c == '\\') {
					cFileContent << "\\\\";
				}
				// For non-printable characters, output as \xNN
				else {
					cFileContent << "\\x"
						<< std::hex << std::uppercase
						<< std::setw(2) << std::setfill('0')
						<< static_cast<int>(c);
	
					if (j + 1 < str.size()) {
						unsigned char next = static_cast<unsigned char>(str[j + 1]);
						if (std::isxdigit(next)) {
							cFileContent << "\"\"";
						}
					}
	
					cFileContent << std::dec;
				}
			}
	
			cFileContent << "\"," << std::endl;
		} else break;
	}
	cFileContent << "};" << std::endl << std::endl;

	for (const auto& [original, minified] : variableMap) {
		if (minified[0] == 'u') {
			cFileContent << "const char* " << original << " = \"" << minified << "\";" << std::endl;
		}
	}

	cFileContent << std::endl;

	cFileContent << getShaderSourceFromFileSrc;

	return cFileContent.str();
}

// Function to generate packed content for shaders
std::vector<uint8_t> generatePackedContent(
	const std::unordered_map<std::string, std::string>& shaders,
	const std::unordered_map<uint16_t, std::string>& tokenList,
	std::vector<std::pair<std::string, size_t>>& shadersOffsets
) {
	std::vector<uint8_t> packedContent;
	size_t currentOffset = 0;

	for (int i = 0; i < tokenList.size(); i++) {
		if (tokenList.find((uint16_t)currentOffset) == tokenList.end()) {
			throw std::runtime_error("Token list malformed");
		}

		std::string token = tokenList.at((uint16_t)currentOffset);
		packedContent.insert(packedContent.end(), token.begin(), token.end());
		packedContent.push_back('\0');
		currentOffset += token.size() + 1;
	}

	for (const auto& [name, text] : shaders) {
		shadersOffsets.push_back({name, currentOffset});
		packedContent.insert(packedContent.end(), text.begin(), text.end());
		packedContent.push_back('\0');
		currentOffset += text.size() + 1;
	}

	return packedContent;
}
