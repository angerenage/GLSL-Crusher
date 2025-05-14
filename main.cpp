#include <iostream>

#include "FileUtils.h"
#include "ShaderUtils.h"
#include "Token.h"
#include "OutputGenerator.h"

// Function to parse command-line arguments
void parseArguments(
	int argc, char** argv,
	std::string& outputPackFile, std::string& outputHeaderFile, std::string& outputCFile,
	int& maxGLSLVersion, bool& useCoreVersion,
	std::vector<std::string>& shaderFiles,
	size_t& minTokenSize, size_t& maxTokenSize,
	bool& verbose
) {
	if (argc < 2) {
		throw std::runtime_error("Usage: " + std::string(argv[0]) + " <shader_file1> <shader_file2> ... [--min-token-size <size>] [--max-token-size <size>] [-p <pack_file> | --output-pack <pack_file>] [-h <header_file> | --output-header <header_file>] [-c <c_file> | --output-c <c_file>] [-v <version> | --glsl-version <version>] [--core | --no-core] [--verbose]");
	}

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if ((arg == "--min-token-size") && i + 1 < argc) {
			minTokenSize = std::stoull(argv[++i]);
		}
		else if ((arg == "--max-token-size") && i + 1 < argc) {
			maxTokenSize = std::stoull(argv[++i]);
		}
		else if ((arg == "-p" || arg == "--output-pack") && i + 1 < argc) {
			outputPackFile = argv[++i];
		}
		else if ((arg == "-h" || arg == "--output-header") && i + 1 < argc) {
			outputHeaderFile = argv[++i];
		}
		else if ((arg == "-c" || arg == "--output-c") && i + 1 < argc) {
			outputCFile = argv[++i];
		}
		else if ((arg == "-v" || arg == "--glsl-version") && i + 1 < argc) {
			maxGLSLVersion = std::stoi(argv[++i]);
		}
		else if (arg == "--core") {
			useCoreVersion = true;
		}
		else if (arg == "--no-core") {
			useCoreVersion = false;
		}
		else if (arg == "--verbose") {
			verbose = true;
		}
		else {
			shaderFiles.push_back(arg);
		}
	}

	if (maxTokenSize != 0 && maxTokenSize < minTokenSize) {
		throw std::runtime_error("Error: --max-token-size must be greater than or equal to --min-token-size.");
	}

	if (shaderFiles.empty()) {
		throw std::runtime_error("Error: No shader files provided.");
	}
}

// Function to process shaders and extract relevant information
void processShaders(
	const std::vector<std::string>& shaderFiles,
	int maxGLSLVersion,
	std::unordered_map<std::string, std::string>& shaders,
	std::unordered_map<std::string, std::string>& globalUniformMap,
	std::unordered_map<std::string, std::string>& globalInOutMap,
	int& highestGLSLVersion,
	bool verbose
) {
	for (const auto& filePath : shaderFiles) {
		std::string shaderCode = readFile(filePath);

		if (verbose) {
			std::cout << "Processing shader: " << filePath << "\n";
		}

		// Extract and validate version
		int shaderVersion = extractGLSLVersion(shaderCode);
		if (shaderVersion > 0) {
			highestGLSLVersion = std::max(highestGLSLVersion, shaderVersion);
			if (maxGLSLVersion > 0 && shaderVersion > maxGLSLVersion) {
				throw std::runtime_error("Error: Shader " + filePath + " uses GLSL version " + std::to_string(shaderVersion) + ", which exceeds the specified maximum version " + std::to_string(maxGLSLVersion) + ".");
			}
		}

		removeGLSLVersionDirective(shaderCode);

		shaders[filePath] = extractExternals(shaderCode, globalUniformMap, globalInOutMap, verbose);
	}
}

int main(int argc, char** argv) {
	try {
		std::string outputPackFile = "shaders.pack";
		std::string outputHeaderFile = "unpacker.h";
		std::string outputCFile = "unpacker.c";
		int maxGLSLVersion = 0; // 0 means auto-detect the highest version
		std::vector<std::string> shaderFiles;
		bool useCoreVersion = true;
		bool verbose = false;
		size_t minTokenSize = 3;
		size_t maxTokenSize = 0;

		// Parse command-line arguments
		parseArguments(argc, argv, outputPackFile, outputHeaderFile, outputCFile, maxGLSLVersion, useCoreVersion, shaderFiles, minTokenSize, maxTokenSize, verbose);

		std::unordered_map<std::string, std::string> shaders;
		std::unordered_map<std::string, std::string> globalUniformMap;
		std::unordered_map<std::string, std::string> globalInOutMap;
		int highestGLSLVersion = 0;

		// Process shaders
		processShaders(shaderFiles, maxGLSLVersion, shaders, globalUniformMap, globalInOutMap, highestGLSLVersion, verbose);

		// Determine the GLSL version to use
		if (maxGLSLVersion == 0) maxGLSLVersion = highestGLSLVersion;

		if (verbose) {
			std::cout << "Highest GLSL version detected: " << highestGLSLVersion << std::endl;
			std::cout << "Using GLSL version: " << maxGLSLVersion << std::endl;
		}

		// Calculate the length of the longest shader
		size_t longestShaderLength = 0;
		for (const auto& shader : shaders) {
			longestShaderLength = std::max(longestShaderLength, shader.second.length());
		}

		Tokens tokens = compress_texts(shaders, minTokenSize, maxTokenSize, verbose);

		// Pass the GLSL version to the header generator
		std::string glslVersionDirective = "#version " + std::to_string(maxGLSLVersion) + (useCoreVersion ? " core" : "");
		std::vector<std::pair<std::string, size_t>> shadersOffsets;

		// Generate the packed content for shaders and write it to the specified file
		std::vector<uint8_t> packedContent = generatePackedContent(shaders, tokens.token_list, shadersOffsets);
		writeFile(outputPackFile, packedContent);
		if (verbose) {
			std::cout << outputPackFile << " generated with size: " << packedContent.size() << " bytes." << std::endl;
		}

		// Generate the header content and write it to the specified file
		std::string header = generateHeader(globalUniformMap, shadersOffsets);
		writeFile(outputHeaderFile, header);
		if (verbose) {
			std::cout << outputHeaderFile << " generated." << std::endl;
		}

		// Generate the C file content and write it to the specified file
		std::string cFileContent = generateCFile(tokens.token_char_map, globalUniformMap, glslVersionDirective, longestShaderLength);
		writeFile(outputCFile, cFileContent);
		if (verbose) {
			std::cout << outputCFile << " generated." << std::endl;
		}
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
