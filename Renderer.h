
#pragma once
#include <vulkan/vulkan.hpp>
#include <SFML/window.hpp>

struct GpuImage
{
	vk::UniqueImage image;
	vk::UniqueImageView view;
	vk::DescriptorImageInfo descriptorInfo;

	vk::UniqueDeviceMemory memory;
	vk::DeviceSize memorySize;

	void* Map(vk::Device device, vk::DeviceSize offset = 0)
	{
		auto mem = device.mapMemory(*memory, offset, memorySize);
		return mem;
	};

	void Unmap(vk::Device device)
	{
		device.unmapMemory(*memory);
	};
};

struct GpuBuffer
{
	vk::UniqueBuffer buffer;
	vk::DescriptorBufferInfo descriptorInfo;
	vk::UniqueDeviceMemory memory;
	vk::DeviceSize memorySize;

	void* Map(vk::Device device, vk::DeviceSize offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE)
	{
		auto mem = device.mapMemory(*memory, offset, size);
		return mem;
	}

	template<typename T>
	void Upload(vk::Device device, T& thing)
	{
		auto mem = Map(device);
		memcpy(mem, &thing, sizeof(T));
		Unmap(device);
	}

	void Unmap(vk::Device device)
	{
		device.unmapMemory(*memory);
	}
};

enum NiftyFlags
{
	CpuMappableMemory = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
};

class Renderer
{
private:
		vk::UniqueCommandBuffer m_cmdBuffer;

public:
		int m_framebufferWidth;
		int m_framebufferHeight;

		vk::UniqueInstance m_instance;
		vk::PhysicalDevice m_gpu;
		vk::Device m_device;
		vk::PhysicalDeviceMemoryProperties m_memoryProps;

		uint32_t m_queueFamilyIndex;
		vk::Queue m_queue;

		vk::UniqueCommandPool m_cmdBufferPool;

		vk::UniqueSurfaceKHR m_presentSurface;
		vk::Format m_swapchainFormat;
		vk::UniqueSwapchainKHR m_swapchain;
		std::vector<vk::Image> m_swapchainImages;
		std::vector<vk::ImageView> m_swapchainImageViews;
		uint32_t m_currentSwapchainIndex;
		vk::Semaphore m_imageAcquireSemaphore;
		vk::Fence m_frameFence;
		GpuImage m_depthBuffer;
		vk::RenderPass m_renderPass;
		vk::Sampler m_linearClampSampler;

		vk::Framebuffer m_framebuffers[2];
		vk::Viewport m_viewport;
		vk::Rect2D m_scissor;

public:
	void CreateSurface(sf::Window * window);
	void CreateVulkanInstance();
	void CreateVulkanDevice();
	void CreateCmdBuffer();
	void CreateSwapchain(sf::Window* window);
	void CreateDepthBuffer();
	void CreateRenderPass();
	void CreateFrameBuffer();
	template<typename T> void AllocateGpuMemory(T& obj, vk::MemoryPropertyFlags memoryFlags);
	vk::CommandBuffer BeginRenderPass();
	void EndRenderPass(vk::CommandBuffer cmdBuffer);
	void SubmitCommandBuffers(vk::CommandBuffer * cmdBuffers, uint32_t commandBufferCount, vk::Fence fence);
	bool WaitForFence(vk::Fence fence, uint64_t timeoutNanos);
	void SetDefaultViewportAndScissor(vk::CommandBuffer cmdBuffer);
	void SetViewportAndScissor(vk::CommandBuffer cmdBuffer, sf::Vector2u size, sf::Vector2u offset);
	void Present();
	GpuBuffer CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryProperties);
	GpuBuffer CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlags, NiftyFlags flags);
	GpuImage CreateImage(uint32_t x, uint32_t y, vk::Format format, vk::ImageTiling tiling, vk::Sampler sampler, vk::ImageUsageFlags usageFlags, vk::MemoryPropertyFlags memoryProperties);
	void CopyBufferToImage(vk::CommandBuffer cmdBuffer, vk::Buffer srcBuffer, vk::Image dstImage, uint32_t x, uint32_t y);
	void TransitionImage(vk::CommandBuffer cmdBuffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	GpuImage UploadTexture(std::byte* texture, int x, int y);
	void Init(sf::Window* window);
};

class Pipeline
{
public:
	vk::Device m_device;

	Pipeline(vk::Device device);

	bool m_ready = false;

	std::vector<vk::DescriptorSetLayoutBinding> m_bindings;
	std::vector<vk::VertexInputBindingDescription> m_vertexBufferBindings;
	std::vector<vk::VertexInputAttributeDescription> m_vertexAttributes;

	vk::UniqueDescriptorSetLayout m_descriptorLayout;
	vk::UniquePipelineLayout m_pipelineLayout;
	vk::UniqueDescriptorPool m_descriptorPool;

	vk::PipelineShaderStageCreateInfo m_vertexStage;
	vk::PipelineShaderStageCreateInfo m_fragmentStage;
	vk::UniqueShaderModule m_vertexModule;
	vk::UniqueShaderModule m_fragmentModule;

	vk::UniquePipeline m_pipeline;
	vk::DescriptorSetLayoutBinding* HasExistingBinding(int bindingPoint);

	void AddVertexBufferBinding(int bindingPoint, int size, vk::VertexInputRate inputRate);
	void AddVertexBufferAttribute(int bindingPoint, int indexInBinding, vk::Format format, int offset);
	void AddVertexBinding(int bindingPoint, int count, vk::DescriptorType type);
	void AddFragmentBinding(int bindingPoint, int count, vk::DescriptorType type);
	void CreateDescriptors(int maxSets);
	void CreateVertexShader(const char* shaderText, const char* shaderName);
	void CreateFragmentShader(const char* shaderText, const char* shaderName);
	void CreatePipeline(vk::RenderPass renderPass, int subpass = 0);
	std::vector<vk::UniqueDescriptorSet> AllocateDescriptorSets(int count = 1);
};
	