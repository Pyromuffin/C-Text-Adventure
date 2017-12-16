


#include <fstream>
#include <assert.h>

#include <vulkan\vulkan.h>
#include <shaderc\shaderc.hpp>

#include "ShaderCompiler.h"

static char fileNameAndPath[1024]; // fix this later
#define SHADER_DIRECTORY "C:\\Users\\pyrom\\Documents\\GitHub\\C-Text-Adventure\\Shaders"

std::vector<uint32_t> CompileShaderFile(const char* fileName, shaderc_shader_kind kind, const char* shaderName)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	//options.SetSourceLanguage(shaderc_source_language_hlsl);

	snprintf(fileNameAndPath, 1024, "\\%s%s", SHADER_DIRECTORY, fileName);

	std::ifstream file(fileNameAndPath, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	if (!file.read(buffer.data(), size))
	{
		assert(false && "Shader file read error!");
	}

	auto result = compiler.CompileGlslToSpv(buffer.data(), kind, shaderName, options);
	if (result.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success)
	{
		printf("Shader error: %s\n", result.GetErrorMessage().c_str());
		return std::vector<uint32_t>();
	}

	return { result.cbegin(), result.cend() };
}


std::vector<uint32_t> CompileShaderString(const char* source, shaderc_shader_kind kind, const char* shaderName)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;

	auto result = compiler.CompileGlslToSpv(source, kind, shaderName, options);
	if (result.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success)
	{
		printf("Shader error: %s\n", result.GetErrorMessage().c_str());
		return std::vector<uint32_t>();
	}


	return { result.cbegin(), result.cend() };
}