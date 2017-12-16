#pragma once

#include <vector>
#include<shaderc\shaderc.hpp>

std::vector<uint32_t> CompileShaderFile(const char* fileName, shaderc_shader_kind kind, const char* shaderName);
std::vector<uint32_t> CompileShaderString(const char* source, shaderc_shader_kind kind, const char* shaderName);
