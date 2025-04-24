cmake_minimum_required(VERSION 3.10)

project(GLSLCrusher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

set(SOURCES
	main.cpp
	OutputGenerator.cpp
	ShaderUtils.cpp
	FileUtils.cpp
	Token.cpp
)

add_executable(GLSLCrusher ${SOURCES})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	if(CMAKE_CXX_COMPILER MATCHES "aarch64.*" OR CMAKE_CXX_COMPILER MATCHES "arm64.*")
		target_compile_options(GLSLCrusher PRIVATE
			$<$<CONFIG:Release>:-O3 -flto>
		)
	else()
		target_compile_options(GLSLCrusher PRIVATE
			$<$<CONFIG:Release>:-O3 -march=native -flto>
		)
	endif()

	target_link_options(GLSLCrusher PRIVATE
		$<$<CONFIG:Release>:-flto>
	)
elseif(MSVC)
	target_compile_options(GLSLCrusher PRIVATE
		$<$<CONFIG:Release>:/O2 /GL>
	)

	target_link_options(GLSLCrusher PRIVATE
		$<$<CONFIG:Release>:/LTCG>
	)
endif()
