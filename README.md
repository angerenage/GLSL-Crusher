# GLSL Crusher

GLSL Crusher is a tool designed to compress and optimize GLSL shaders by combining multiple shader files into a single packed file. It ensures consistent renaming of external variables and generates a C header and source file to facilitate shader usage in applications.

Note that GLSL Crusher does not perform shader minification, so it is recommended to use it alongside other tools for full optimization.

## Installation

1. **Clone the Repository**
	```bash
	git clone https://github.com/angerenage/GLSL-Crusher.git
	cd GLSL-Crusher
	```

2. **Install Dependencies**
	Ensure you have the following installed on your system:
	- CMake (version 3.15 or higher)
	- A C++ compiler supporting C++20 (e.g., g++, MSVC)

3. **Build the Project**
	```bash
	mkdir build
	cd build
	cmake ..
	cmake --build . --config Release
	```

## Usage

1. **Run the Tool**
	After building, the executable will be located in the `build/bin` directory. You can run it as follows:
	```bash
	./bin/GLSLcrusher <shader_file1> <shader_file2> ... [options]
	```

2. **Command-Line Arguments**
	- `<shader_file1> <shader_file2> ...`: One or more GLSL shader files to be processed.
	- `--min-token-size <size>`: Specify the minimum token size for compression. Default is 3.
	- `--max-token-size <size>`: Specify the maximum token size for compression.
	- `-p <pack_file>` or `--output-pack <pack_file>`: Specify the output file for the packed shaders. Default is `shaders.pack`.
	- `-h <header_file>` or `--output-header <header_file>`: Specify the output file for the generated header. Default is `unpacker.h`.
	- `-c <c_file>` or `--output-c <c_file>`: Specify the C file for the unpacker function. Default is `unpacker.c`.
	- `-v <version>` or `--glsl-version <version>`: Specify the maximum GLSL version to use. If not provided, the highest version detected in the shaders will be used.
	- `--core`: Use the core profile for GLSL (default).
	- `--no-core`: Do not use the core profile for GLSL.
	- `--verbose`: Enable verbose logging for debugging purposes.

3. **Example**
	```bash
	./bin/GLSLcrusher shaders/example_shader1.glsl shaders/example_shader2.glsl -p output.pack -h output.h -v 450 --core --verbose
	```
	This will process `example_shader1.glsl` and `example_shader2.glsl`, output the packed shaders to `output.pack`, generate a header file `output.h`, generate a source file `unpacker.c` and use GLSL version 450 with the core profile, while enabling verbose logging.

## Output

- **Packed File**: A single file containing all the compressed shaders.
- **Header File**: A C header file with:
	- Metadata about the shaders, including their offsets in the packed file.
- **C File**: A C source file containing a function to decompress the shaders at runtime.
	- The names of external variables (e.g., uniforms, inputs, outputs).
	- A function to decompress the shaders for use in your application.

## Contributing

Contributions are welcome! Feel free to open issues or submit pull requests to improve the tool.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.