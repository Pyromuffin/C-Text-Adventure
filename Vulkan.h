
#pragma once

#include <vulkan/vulkan.hpp>

class sf::Window;

void InitVulkan(sf::Window* window, unsigned char* texture, int x, int y);
void RenderFrame(sf::Window* window);

struct GpuImage
{
	vk::Image image;
	vk::ImageView view;
	vk::DescriptorImageInfo descriptorInfo;

	vk::DeviceMemory memory;
	vk::DeviceSize memorySize;

	void* Map(vk::Device device, vk::DeviceSize offset = 0)
	{
		return device.mapMemory(memory, offset, memorySize);
	}

	void Unmap(vk::Device device)
	{
		device.unmapMemory(memory);
	}
};

struct GpuBuffer
{
	vk::Buffer buffer;
	vk::DescriptorBufferInfo descriptorInfo;
	vk::DeviceMemory memory;
	vk::DeviceSize memorySize;

	void* Map(vk::Device device, vk::DeviceSize offset = 0)
	{
		return device.mapMemory(memory, offset, memorySize);
	}

	void Unmap(vk::Device device)
	{
		device.unmapMemory(memory);
	}
};



class Renderer
{
public:
		int m_framebufferWidth;
		int m_framebufferHeight;

		vk::Instance m_instance;
		vk::PhysicalDevice m_gpu;
		vk::Device m_device;
		vk::PhysicalDeviceMemoryProperties m_memoryProps;

		uint32_t m_queueFamilyIndex;
		vk::Queue m_queue;

		vk::CommandPool m_cmdBufferPool;
		vk::CommandBuffer m_cmdBuffer; // perhaps we dont want this? idk yet.

		vk::SurfaceKHR m_presentSurface;
		vk::Format m_swapchainFormat;
		vk::SwapchainKHR m_swapchain;
		std::vector<vk::Image> m_swapchainImages;
		std::vector<vk::ImageView> m_swapchainImageViews;
		uint32_t m_currentSwapchainIndex;
		vk::Semaphore m_imageAcquireSemaphore;
		vk::Fence m_frameFence;
		GpuImage m_depthBuffer;

		vk::Framebuffer m_framebuffers[2];
		vk::Viewport m_viewport;
		vk::Rect2D m_scissor;

public:
		void CreatePipeline();
		VkCommandBuffer BeginRenderPass();
		void EndRenderPass(VkCommandBuffer cmdBuffer);
		void SubmitCommandBuffers(VkCommandBuffer * cmdBuffers, uint32_t commandBufferCount, VkFence fence);
		bool WaitForFence(VkFence fence, uint64_t timeoutNanos);
		void SetViewportAndScissor(VkCommandBuffer cmdBuffer, sf::Vector2u size, sf::Vector2u offset);
		void Present();
		GpuBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryProperties);
		GpuImage CreateImage(uint32_t x, uint32_t y, VkFormat format, VkImageTiling tiling, VkSampler sampler, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryProperties);
		void CopyBufferToImage(VkCommandBuffer cmdBuffer, VkBuffer srcBuffer, VkImage dstImage, uint32_t x, uint32_t y);
		void TransitionImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
};

class Pass
{
	bool m_ready;

	std::vector<vk::DescriptorSetLayoutBinding> m_bindings;
	int m_uniqueBindings = 0;
	//std::vector<vk::DescriptorSetLayoutBinding> m_vertexBindings;
	//std::vector<vk::DescriptorSetLayoutBinding> m_fragmentBindings;



	vk::VertexInputBindingDescription m_vertexBindingDesc;
	vk::VertexInputAttributeDescription m_vertexAttributes[2];

	vk::UniqueDescriptorSetLayout m_descriptorLayout;
	vk::UniquePipelineLayout m_pipelineLayout;
	vk::UniqueDescriptorPool m_descriptorPool;
	vk::RenderPass m_renderPass;

	vk::PipelineShaderStageCreateInfo m_vertexStage;
	vk::PipelineShaderStageCreateInfo m_fragmentStage;
	vk::Pipeline m_pipeline;


	bool IsBindingUnique(int bindingPoint)
	{
		for (auto& binding : m_bindings)
		{
			if (bindingPoint == binding.binding)
				return false;
		}

		return true;
	}

	void AddVertexBinding(int bindingPoint, int count, vk::DescriptorType type)
	{
		vk::DescriptorSetLayoutBinding vertex_binding;
		vertex_binding.binding = bindingPoint;
		vertex_binding.descriptorType = type;
		vertex_binding.descriptorCount = count;
		vertex_binding.stageFlags = vk::ShaderStageFlagBits::eVertex;
		vertex_binding.pImmutableSamplers = NULL;

		 m_bindings.push_back(vertex_binding);
		 if (IsBindingUnique(bindingPoint))
			 m_uniqueBindings++;
		
	};

	void AddFragmentBinding(int bindingPoint, int count, vk::DescriptorType type)
	{
		vk::DescriptorSetLayoutBinding fragment_binding;
		fragment_binding.binding = bindingPoint;
		fragment_binding.descriptorType = type;
		fragment_binding.descriptorCount = count;
		fragment_binding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		fragment_binding.pImmutableSamplers = NULL;
		
		m_bindings.push_back(fragment_binding);
		if (IsBindingUnique(bindingPoint))
			m_uniqueBindings++;
	};


	void CreateDescriptors(vk::Device device, int maxSets)
	{
		vk::DescriptorSetLayoutCreateInfo descriptor_layout;
		descriptor_layout.bindingCount = m_bindings.size();
		descriptor_layout.pBindings = m_bindings.data();
		m_descriptorLayout = device.createDescriptorSetLayoutUnique(descriptor_layout);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorLayout.get();

		m_pipelineLayout = device.createPipelineLayoutUnique(pipelineLayoutInfo);

		std::vector<vk::DescriptorPoolSize> poolSizes(m_bindings.size());
		for (auto& binding : m_bindings)
		{
			vk::DescriptorPoolSize size{ binding.descriptorType, binding.descriptorCount }; // THIS IS WRONG, WILL DOUBLE COUNT NON-UNIQUE BINDINGS!
			poolSizes.push_back(size);
		}

		vk::DescriptorPoolCreateInfo descriptor_pool = {};
		descriptor_pool.maxSets = maxSets;
		descriptor_pool.poolSizeCount = poolSizes.size();
		descriptor_pool.pPoolSizes = poolSizes.data();

		m_descriptorPool = device.createDescriptorPoolUnique(descriptor_pool);
	}

	void CreatePipeline(vk::Device device)
	{
		vk::DynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
		vk::PipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.pDynamicStates = dynamicStateEnables;
		dynamicState.dynamicStateCount = 0;

		vk::PipelineVertexInputStateCreateInfo vi;
		vi.vertexBindingDescriptionCount = m_vertexBindings.size();
		vi.pVertexBindingDescriptions = m_vertexBindings.data();
		vi.vertexAttributeDescriptionCount = 2;
		vi.pVertexAttributeDescriptions = m_vertexAttributes;

		VkPipelineInputAssemblyStateCreateInfo ia;
		ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ia.pNext = NULL;
		ia.flags = 0;
		ia.primitiveRestartEnable = VK_FALSE;
		ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineRasterizationStateCreateInfo rs;
		rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rs.pNext = NULL;
		rs.flags = 0;
		rs.polygonMode = VK_POLYGON_MODE_FILL;
		rs.cullMode = VK_CULL_MODE_BACK_BIT;
		rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rs.depthClampEnable = VK_FALSE;
		rs.rasterizerDiscardEnable = VK_FALSE;
		rs.depthBiasEnable = VK_FALSE;
		rs.depthBiasConstantFactor = 0;
		rs.depthBiasClamp = 0;
		rs.depthBiasSlopeFactor = 0;
		rs.lineWidth = 1.0f;

		VkPipelineColorBlendStateCreateInfo cb;
		cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		cb.pNext = NULL;
		cb.flags = 0;
		VkPipelineColorBlendAttachmentState att_state[1];
		att_state[0].colorWriteMask = 0xf;
		att_state[0].blendEnable = VK_FALSE;
		att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
		att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
		att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		cb.attachmentCount = 1;
		cb.pAttachments = att_state;
		cb.logicOpEnable = VK_FALSE;
		cb.logicOp = VK_LOGIC_OP_NO_OP;
		cb.blendConstants[0] = 1.0f;
		cb.blendConstants[1] = 1.0f;
		cb.blendConstants[2] = 1.0f;
		cb.blendConstants[3] = 1.0f;

		VkPipelineViewportStateCreateInfo vp = {};
		vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vp.pNext = NULL;
		vp.flags = 0;
		vp.viewportCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
		vp.scissorCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
		vp.pScissors = NULL;
		vp.pViewports = NULL;

		VkPipelineDepthStencilStateCreateInfo ds;
		ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.pNext = NULL;
		ds.flags = 0;
		ds.depthTestEnable = VK_TRUE;
		ds.depthWriteEnable = VK_TRUE;
		ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		ds.depthBoundsTestEnable = VK_FALSE;
		ds.minDepthBounds = 0;
		ds.maxDepthBounds = 0;
		ds.stencilTestEnable = VK_FALSE;
		ds.back.failOp = VK_STENCIL_OP_KEEP;
		ds.back.passOp = VK_STENCIL_OP_KEEP;
		ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
		ds.back.compareMask = 0;
		ds.back.reference = 0;
		ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
		ds.back.writeMask = 0;
		ds.front = ds.back;

		VkPipelineMultisampleStateCreateInfo ms;
		ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ms.pNext = NULL;
		ms.flags = 0;
		ms.pSampleMask = NULL;
		ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		ms.sampleShadingEnable = VK_FALSE;
		ms.alphaToCoverageEnable = VK_FALSE;
		ms.alphaToOneEnable = VK_FALSE;
		ms.minSampleShading = 0.0;

		VkGraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = NULL;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = 0;
		pipelineInfo.flags = 0;
		pipelineInfo.pVertexInputState = &vi;
		pipelineInfo.pInputAssemblyState = &ia;
		pipelineInfo.pRasterizationState = &rs;
		pipelineInfo.pColorBlendState = &cb;
		pipelineInfo.pTessellationState = NULL;
		pipelineInfo.pMultisampleState = &ms;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.pViewportState = &vp;
		pipelineInfo.pDepthStencilState = &ds;

		VkPipelineShaderStageCreateInfo stages[] = { m_vertexStage, m_fragmentStage };
		pipelineInfo.pStages = stages;
		pipelineInfo.stageCount = 2;
		pipelineInfo.renderPass = m_renderPass;
		pipelineInfo.subpass = 0;

		VK_CHECK(vkCreateGraphicsPipelines(m_device, NULL, 1, &pipelineInfo, NULL, &m_pipeline));









	};

	std::vector<vk::UniqueDescriptorSet> AllocateDescriptorSets(vk::Device device, int count = 1)
	{
		vk::DescriptorSetAllocateInfo alloc_info;
		alloc_info.descriptorPool = m_descriptorPool.get();
		alloc_info.descriptorSetCount = count;
		alloc_info.pSetLayouts = &m_descriptorLayout.get();
		return device.allocateDescriptorSetsUnique(alloc_info);
	}

};
	