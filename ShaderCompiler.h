#pragma once

#include <vector>
#include<shaderc\shaderc.hpp>

void BeginCompilation();
void EndCompilation();
std::vector<uint32_t> CompileShader(const std::string& source, shaderc_shader_kind kind, const char* shaderName);
