#pragma once

#include <vector>
#include <vulkan/vulkan.h>

void BeginCompilation();
void EndCompilation();
bool GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *pshader, std::vector<unsigned int> &spirv);
