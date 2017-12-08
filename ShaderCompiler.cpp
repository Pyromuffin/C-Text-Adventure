


#include <string>
#include <fstream>
#include <sstream>

#include <vulkan\vulkan.h>
#include <shaderc\shaderc.hpp>

#include "ShaderCompiler.h"

std::vector<uint32_t> CompileShader(const std::string& source, shaderc_shader_kind kind, const char* shaderName)
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