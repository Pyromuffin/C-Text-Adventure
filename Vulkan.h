
#pragma once

#include <vulkan/vulkan.hpp>

class sf::Window;

void InitVulkan(sf::Window* window, unsigned char* texture, int x, int y);
void RenderFrame(sf::Window* window);

struct GpuImage
{
	VkImage image;
	VkImageView view;
	VkDescriptorImageInfo descriptorInfo;

	VkDeviceMemory memory;
	VkDeviceSize memorySize;
};

struct GpuBuffer
{
	VkBuffer buffer;
	VkDescriptorBufferInfo descriptorInfo;

	VkDeviceMemory memory;
	VkDeviceSize memorySize;
};

class Renderer
{
public:
		int m_framebufferWidth;
		int m_framebufferHeight;

		VkInstance m_instance;
		VkPhysicalDevice m_gpu;
		VkDevice m_device;
		VkPhysicalDeviceMemoryProperties m_memoryProps;

		uint32_t m_queueFamilyIndex;
		VkQueue m_queue;

		VkCommandPool m_cmdBufferPool;
		VkCommandBuffer m_cmdBuffer;

		VkSurfaceKHR m_presentSurface;
		VkFormat m_swapchainFormat;
		VkSwapchainKHR m_swapchain;
		std::vector<VkImage> m_swapchainImages;
		std::vector<VkImageView> m_swapchainImageViews;
		uint32_t m_currentSwapchainIndex;
		VkSemaphore m_imageAcquireSemaphore;
		VkFence m_frameFence;

		GpuImage m_depthBuffer;
		GpuBuffer m_uniformBuffer;

		std::vector<VkDescriptorSetLayout> m_descriptorLayouts;
		std::vector<VkDescriptorSet> m_descriptorSets;
		VkPipelineLayout m_pipelineLayout;
		VkDescriptorPool m_descriptorPool;
		VkSampler m_linearClampSampler;

		VkRenderPass m_renderPass;

		VkPipelineShaderStageCreateInfo m_vertexStage;
		VkPipelineShaderStageCreateInfo m_fragmentStage;

		VkFramebuffer m_framebuffers[2];
		VkViewport m_viewport;
		VkRect2D m_scissor;

		GpuBuffer m_vertexBuffer;
		VkVertexInputBindingDescription m_vertexBinding;
		VkVertexInputAttributeDescription m_vertexAttributes[2];

		VkPipeline m_pipeline;

public:
		void CreatePipeline();
		VkCommandBuffer BeginRenderPass();
		void EndRenderPass(VkCommandBuffer cmdBuffer);
		void SubmitCommandBuffers(VkCommandBuffer * cmdBuffers, uint32_t commandBufferCount, VkFence fence);
		bool WaitForFence(VkFence fence, uint64_t timeoutNanos);
		void SetViewportAndScissor(VkCommandBuffer cmdBuffer, sf::Vector2u size, sf::Vector2u offset);
		void Present();
};