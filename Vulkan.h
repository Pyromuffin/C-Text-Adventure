
#pragma once
#include <optional>
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

class Pipeline
{
	vk::Device m_device;

	Pipeline(vk::Device device)
	{
		m_device = device;
	}

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


	std::optional<vk::DescriptorSetLayoutBinding&> HasExistingBinding(int bindingPoint)
	{
		for (auto& binding : m_bindings)
		{
			if (bindingPoint == binding.binding)
			{
				return std::make_optional(binding);
			}
		}

		return std::nullopt;
	}


	void AddVertexBufferBinding(int bindingPoint, int size, vk::VertexInputRate inputRate)
	{
		m_vertexBufferBindings.push_back( vk::VertexInputBindingDescription(bindingPoint, size, inputRate) );
	}

	void AddVertexBufferAttribute(int bindingPoint, int indexInBinding, vk::Format format, int offset)
	{
		m_vertexAttributes.push_back(vk::VertexInputAttributeDescription(indexInBinding, bindingPoint, format, offset));
	}

	void AddVertexBinding(int bindingPoint, int count, vk::DescriptorType type)
	{
		auto maybeBinding = HasExistingBinding(bindingPoint);
		if(maybeBinding)
		{
			auto& binding = *maybeBinding;
			assert(binding.descriptorType == type);
			assert(binding.descriptorCount == count);
			binding.stageFlags |= vk::ShaderStageFlagBits::eVertex;
		}
		else
		{
			vk::DescriptorSetLayoutBinding vertex_binding;
			vertex_binding.binding = bindingPoint;
			vertex_binding.descriptorType = type;
			vertex_binding.descriptorCount = count;
			vertex_binding.stageFlags = vk::ShaderStageFlagBits::eVertex;
			vertex_binding.pImmutableSamplers = NULL;
			m_bindings.push_back(vertex_binding);
		}
	};

	void AddFragmentBinding(int bindingPoint, int count, vk::DescriptorType type)
	{
		auto maybeBinding = HasExistingBinding(bindingPoint);
		if (maybeBinding)
		{
			auto& binding = *maybeBinding;
			assert(binding.descriptorType == type);
			assert(binding.descriptorCount == count);
			binding.stageFlags |= vk::ShaderStageFlagBits::eFragment;
		}
		else
		{
			vk::DescriptorSetLayoutBinding vertex_binding;
			vertex_binding.binding = bindingPoint;
			vertex_binding.descriptorType = type;
			vertex_binding.descriptorCount = count;
			vertex_binding.stageFlags = vk::ShaderStageFlagBits::eFragment;
			vertex_binding.pImmutableSamplers = NULL;
			m_bindings.push_back(vertex_binding);
		}
	};


	void CreateDescriptors(int maxSets)
	{
		vk::DescriptorSetLayoutCreateInfo descriptor_layout;
		descriptor_layout.bindingCount = m_bindings.size();
		descriptor_layout.pBindings = m_bindings.data();
		m_descriptorLayout = m_device.createDescriptorSetLayoutUnique(descriptor_layout);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorLayout.get();

		m_pipelineLayout = m_device.createPipelineLayoutUnique(pipelineLayoutInfo);

		std::vector<vk::DescriptorPoolSize> poolSizes(m_bindings.size());
		for (auto& binding : m_bindings)
		{
			vk::DescriptorPoolSize size{ binding.descriptorType, binding.descriptorCount };
			poolSizes.push_back(size);
		}

		vk::DescriptorPoolCreateInfo descriptor_pool = {};
		descriptor_pool.maxSets = maxSets;
		descriptor_pool.poolSizeCount = poolSizes.size();
		descriptor_pool.pPoolSizes = poolSizes.data();

		m_descriptorPool = m_device.createDescriptorPoolUnique(descriptor_pool);
	}

	void CreateVertexShader(const char* shaderText, const char* shaderName)
	{
		auto spv = CompileShaderString(shaderText, shaderc_glsl_vertex_shader, shaderName);
		vk::ShaderModuleCreateInfo info;
		info.pCode = spv.data();
		info.codeSize = spv.size() * sizeof(uint32_t);

		m_vertexModule = m_device.createShaderModuleUnique(info);
		
		m_vertexStage.module = m_vertexModule.get();
		m_vertexStage.pName = shaderName;
		m_vertexStage.stage = vk::ShaderStageFlagBits::eVertex;
	}
	
	void CreateFragmentShader(const char* shaderText, const char* shaderName)
	{
		auto spv = CompileShaderString(shaderText, shaderc_glsl_vertex_shader, shaderName);
		vk::ShaderModuleCreateInfo info;
		info.pCode = spv.data();
		info.codeSize = spv.size() * sizeof(uint32_t);

		m_fragmentModule = m_device.createShaderModuleUnique(info);

		m_fragmentStage.module = m_fragmentModule.get();
		m_fragmentStage.pName = shaderName;
		m_fragmentStage.stage = vk::ShaderStageFlagBits::eFragment;
	}


	void CreatePipeline(vk::RenderPass renderPass, int subpass = 0)
	{
		vk::DynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
		vk::PipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.pDynamicStates = dynamicStateEnables;
		dynamicState.dynamicStateCount = 0;

		vk::PipelineVertexInputStateCreateInfo vi;
		vi.vertexBindingDescriptionCount = m_vertexBufferBindings.size();
		vi.pVertexBindingDescriptions = m_vertexBufferBindings.data();
		vi.vertexAttributeDescriptionCount = m_vertexAttributes.size();
		vi.pVertexAttributeDescriptions = m_vertexAttributes.data();

		vk::PipelineInputAssemblyStateCreateInfo ia;
		ia.primitiveRestartEnable = VK_FALSE;
		ia.topology = vk::PrimitiveTopology::eTriangleList;

		vk::PipelineRasterizationStateCreateInfo rs;
		rs.polygonMode = vk::PolygonMode::eFill;
		rs.cullMode = vk::CullModeFlagBits::eBack;
		rs.frontFace = vk::FrontFace::eClockwise;
		rs.depthClampEnable = VK_FALSE;
		rs.rasterizerDiscardEnable = VK_FALSE;
		rs.depthBiasEnable = VK_FALSE;
		rs.depthBiasConstantFactor = 0;
		rs.depthBiasClamp = 0;
		rs.depthBiasSlopeFactor = 0;
		rs.lineWidth = 1.0f;

		vk::PipelineColorBlendStateCreateInfo cb; // we will probably want alpha blending at some point.

		vk::PipelineColorBlendAttachmentState att_state[1];
		att_state[0].colorWriteMask = (vk::ColorComponentFlagBits)0xf;
		att_state[0].blendEnable = VK_FALSE;
		att_state[0].alphaBlendOp = vk::BlendOp::eAdd;
		att_state[0].colorBlendOp = vk::BlendOp::eAdd;
		att_state[0].srcColorBlendFactor = vk::BlendFactor::eZero;
		att_state[0].dstColorBlendFactor = vk::BlendFactor::eZero;
		att_state[0].srcAlphaBlendFactor = vk::BlendFactor::eZero;
		att_state[0].dstAlphaBlendFactor = vk::BlendFactor::eZero;
		cb.attachmentCount = 1;
		cb.pAttachments = att_state;
		cb.logicOpEnable = VK_FALSE;
		cb.logicOp = vk::LogicOp::eNoOp;
		cb.blendConstants[0] = 1.0f;
		cb.blendConstants[1] = 1.0f;
		cb.blendConstants[2] = 1.0f;
		cb.blendConstants[3] = 1.0f;

		vk::PipelineViewportStateCreateInfo vp = {};
		vp.viewportCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = vk::DynamicState::eViewport;
		vp.scissorCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = vk::DynamicState::eScissor;
		vp.pScissors = NULL;
		vp.pViewports = NULL;

		vk::PipelineDepthStencilStateCreateInfo ds; 
		ds.depthTestEnable = VK_TRUE;
		ds.depthWriteEnable = VK_TRUE;
		ds.depthCompareOp = vk::CompareOp::eLessOrEqual;
		ds.depthBoundsTestEnable = VK_FALSE;
		ds.minDepthBounds = 0;
		ds.maxDepthBounds = 0;
		ds.stencilTestEnable = VK_FALSE;
		ds.back.failOp = vk::StencilOp::eKeep;
		ds.back.passOp = vk::StencilOp::eKeep;
		ds.back.compareOp = vk::CompareOp::eAlways;
		ds.back.compareMask = 0;
		ds.back.reference = 0;
		ds.back.depthFailOp = vk::StencilOp::eKeep;;
		ds.back.writeMask = 0;
		ds.front = ds.back;

		vk::PipelineMultisampleStateCreateInfo ms; // same
		ms.pSampleMask = NULL;
		ms.rasterizationSamples = vk::SampleCountFlagBits::e1;
		ms.sampleShadingEnable = VK_FALSE;
		ms.alphaToCoverageEnable = VK_FALSE;
		ms.alphaToOneEnable = VK_FALSE;
		ms.minSampleShading = 0.0;

		vk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.layout = m_pipelineLayout.get();
		pipelineInfo.basePipelineHandle = vk::Pipeline(); // null handle!
		pipelineInfo.basePipelineIndex = 0;
		pipelineInfo.pVertexInputState = &vi;
		pipelineInfo.pInputAssemblyState = &ia;
		pipelineInfo.pRasterizationState = &rs;
		pipelineInfo.pColorBlendState = &cb;
		pipelineInfo.pTessellationState = NULL;
		pipelineInfo.pMultisampleState = &ms;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.pViewportState = &vp;
		pipelineInfo.pDepthStencilState = &ds;

		vk::PipelineShaderStageCreateInfo stages[] = { m_vertexStage, m_fragmentStage };
		pipelineInfo.pStages = stages;
		pipelineInfo.stageCount = 2;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = subpass;

		m_pipeline = m_device.createGraphicsPipelineUnique(nullptr, pipelineInfo);

		m_ready = true;
	};

	std::vector<vk::UniqueDescriptorSet> AllocateDescriptorSets(int count = 1)
	{
		vk::DescriptorSetAllocateInfo alloc_info;
		alloc_info.descriptorPool = m_descriptorPool.get();
		alloc_info.descriptorSetCount = count;
		alloc_info.pSetLayouts = &m_descriptorLayout.get();
		return m_device.allocateDescriptorSetsUnique(alloc_info);
	}

};
	