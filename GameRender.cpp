
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GameRender.h"

#include "Renderer.h"
#include "TextProcessing.h"


GameRender::GameRender(Renderer& r) :
	renderer(r),
	regularPipeline(r.m_device)
{

}

void GameRender::CreateVertexBuffer()
{
	vertexBuffer = renderer.CreateBuffer(sizeof(Vert) * maxVertices, vk::BufferUsageFlagBits::eVertexBuffer, CpuMappableMemory);
}

void GameRender::CreateUniformBuffer()
{
	uniformBuffer = renderer.CreateBuffer(sizeof(glm::mat4x4), vk::BufferUsageFlagBits::eUniformBuffer, NiftyFlags::CpuMappableMemory);
}

void GameRender::UpdateUniformBuffer()
{
	auto Projection = glm::ortho(0.0f, (float)renderer.m_framebufferWidth, 0.0f, (float)renderer.m_framebufferHeight);
	auto View = glm::lookAt(
		glm::vec3(0, 0, -10), // Camera is at (-5,3,-10), in World Space
		glm::vec3(0, 0, 0),    // and looks at the origin
		glm::vec3(0, 1, 0)    // Head is up
	);
	// Vulkan clip space has inverted Y and half Z.
	auto Clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f);

	auto MVP = Clip * Projection * View;
	uniformBuffer.Upload(renderer.m_device, MVP);
}

void GameRender::ResetVertexBuffer()
{
	vertIndex = 0;
}

void GameRender::DrawVerts(vk::CommandBuffer cmdBuffer, Vert* verts, int vertCount)
{
	assert(vertCount + vertIndex < maxVertices);

	vk::DeviceSize offset = vertIndex * sizeof(Vert);
	vk::DeviceSize size = vertCount * sizeof(Vert);

	auto mem = vertexBuffer.Map(renderer.m_device, offset, size);
	memcpy(mem, verts, size);
	vertexBuffer.Unmap(renderer.m_device);
	cmdBuffer.draw(vertCount, 1, vertIndex, 0);

	vertIndex += vertCount;
}

void GameRender::Init(std::byte* ralewayBitmap, std::byte* inconsolataBitmap, int x, int y)
{
	regularPipeline.AddVertexBufferBinding(0, sizeof(Vert), vk::VertexInputRate::eVertex);
	regularPipeline.AddVertexBufferAttribute(0, 0, vk::Format::eR32G32Sfloat, 0);					// pos
	regularPipeline.AddVertexBufferAttribute(0, 1, vk::Format::eR32G32Sfloat, sizeof(float) * 2);	// uv

	// vertex buffer binding points are different from shader binding points i hope.

	regularPipeline.AddVertexBinding(0, 1, vk::DescriptorType::eUniformBuffer);
	regularPipeline.AddFragmentBinding(1, 1, vk::DescriptorType::eCombinedImageSampler);

	regularPipeline.CreateDescriptors(10); // really just need one for each font.
	descriptors = regularPipeline.AllocateDescriptorSets(2);

	static const char *vertShaderText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout (std140, binding = 0) uniform buf {\n"
		"        mat4 mvp;\n"
		"} ubuf;\n"
		"layout (location = 0) in vec2 pos;\n"
		"layout (location = 1) in vec2 inTexCoords;\n"
		"layout (location = 0) out vec2 texcoord;\n"
		"void main() {\n"
		"   vec4 position = vec4(pos.xy,0,1);\n"
		"   texcoord = inTexCoords;\n"
		"   gl_Position = ubuf.mvp * position;\n"
		"}\n";

	static const char *fragShaderText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout (binding = 1) uniform sampler2D tex;\n"
		"layout (location = 0) in vec2 texcoord;\n"
		"layout (location = 0) out vec4 outColor;\n"
		"void main() {\n"
		"   outColor = textureLod(tex, texcoord, 0.0);\n"
		"}\n";


	regularPipeline.CreateVertexShader(vertShaderText, "Regular Vertex Shader");
	regularPipeline.CreateFragmentShader(fragShaderText, "Regular Fragment Shader");
	regularPipeline.CreatePipeline(renderer.m_renderPass);

	ralewayFont = renderer.UploadTexture(ralewayBitmap, renderer.m_framebufferWidth, renderer.m_framebufferHeight);
	inconsolataFont = renderer.UploadTexture(inconsolataBitmap, renderer.m_framebufferWidth, renderer.m_framebufferHeight);

	CreateUniformBuffer();
	CreateVertexBuffer();

	vk::WriteDescriptorSet writes[4];
	writes[0].dstSet = *descriptors[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = vk::DescriptorType::eUniformBuffer;
	writes[0].pBufferInfo = &uniformBuffer.descriptorInfo;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;

	writes[1].dstSet = *descriptors[0];
	writes[1].descriptorCount = 1;
	writes[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	writes[1].pImageInfo = &ralewayFont.descriptorInfo;
	writes[1].dstArrayElement = 0;
	writes[1].dstBinding = 1;

	writes[2].dstSet = *descriptors[1];
	writes[2].descriptorCount = 1;
	writes[2].descriptorType = vk::DescriptorType::eUniformBuffer;
	writes[2].pBufferInfo = &uniformBuffer.descriptorInfo;
	writes[2].dstArrayElement = 0;
	writes[2].dstBinding = 0;

	writes[3].dstSet = *descriptors[1];
	writes[3].descriptorCount = 1;
	writes[3].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	writes[3].pImageInfo = &inconsolataFont.descriptorInfo;
	writes[3].dstArrayElement = 0;
	writes[3].dstBinding = 1;

	vkUpdateDescriptorSets(renderer.m_device, 4, (VkWriteDescriptorSet*)writes, 0, NULL);


}


void GameRender::RenderFrame()
{
	auto cmdBuffer = renderer.BeginRenderPass();
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *regularPipeline.m_pipeline);
	// bind raleway
	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *regularPipeline.m_pipelineLayout, 0, 1, &descriptors[0].get(), 0, nullptr);

	const VkDeviceSize offsets[1] = { 0 };
	cmdBuffer.bindVertexBuffers(0, 1, &vertexBuffer.buffer.get(), offsets);


}