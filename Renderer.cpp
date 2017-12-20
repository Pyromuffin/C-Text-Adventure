


#include <cstdio>

#ifdef _WIN32
	#include <Windows.h>
	#ifndef VK_USE_PLATFORM_WIN32_KHR
		#define VK_USE_PLATFORM_WIN32_KHR
	#endif
#endif

#include <SFML/Window.hpp>
#include <SFML\Graphics.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan\vk_layer.h>

#include "ShaderCompiler.h"
#include "Renderer.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>



static VkResult __s_debugResult;
#define VK_CHECK( func ) \
__s_debugResult = func; \
assert(__s_debugResult == VK_SUCCESS)


void init_instance_extension_names( std::vector<const char*>& extensionNames ) {
#ifdef __ANDROID__
	extensionNames.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_WIN32)
	extensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	extensionNames.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	extensionNames.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	iextensionNames.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#else
	extensionNames.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

	extensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
}


void Renderer::CreateSurface(sf::Window* window)
{
VkResult res = VK_SUCCESS;

#ifdef _WIN32
	auto handle = window->getSystemHandle();
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(handle, GWLP_HINSTANCE);

	vk::Win32SurfaceCreateInfoKHR createInfo;
	createInfo.hinstance = hInstance;
	createInfo.hwnd = handle;

	m_presentSurface = m_instance->createWin32SurfaceKHRUnique(createInfo);

#elif defined(__ANDROID__)
	GET_INSTANCE_PROC_ADDR(info.inst, CreateAndroidSurfaceKHR);

	VkAndroidSurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.window = AndroidGetApplicationWindow();
	res = info.fpCreateAndroidSurfaceKHR(info.inst, &createInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	VkWaylandSurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.display = info.display;
	createInfo.surface = info.window;
	res = vkCreateWaylandSurfaceKHR(info.inst, &createInfo, NULL, &info.surface);
#else
	VkXcbSurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.connection = info.connection;
	createInfo.window = info.window;
	res = vkCreateXcbSurfaceKHR(info.inst, &createInfo, NULL, &info.surface);
#endif  // _WIN32

	assert(res == VK_SUCCESS);
}

#define VALIDATION_LAYER_NAME "VK_LAYER_LUNARG_standard_validation"

bool CheckValidationLayerSupport(std::vector<const char*>& validationLayers) {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) 
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) 
		{
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) 
		{
			return false;
		}
	}

	return true;
}

void Renderer::CreateVulkanInstance()
{
	std::vector<const char*> instanceExtensions;
	init_instance_extension_names(instanceExtensions);
	instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

	std::vector<const char*> instanceLayers;
	instanceLayers.push_back(VALIDATION_LAYER_NAME);

	vk::InstanceCreateInfo info;
	info.enabledLayerCount = instanceLayers.size();
	info.ppEnabledLayerNames = instanceLayers.data();
	info.enabledExtensionCount = instanceExtensions.size();
	info.ppEnabledExtensionNames = instanceExtensions.data();

	vk::ApplicationInfo appInfo;
	appInfo.pApplicationName = "C Text Adventure";
	appInfo.pEngineName = "SUPER ENGINE NAME";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	info.pApplicationInfo = &appInfo;

	if (!CheckValidationLayerSupport(instanceLayers)) 
	{
		assert(false && "validation busted");
	}

	m_instance = vk::createInstanceUnique(info);
}


void Renderer::CreateVulkanDevice()
{
	std::vector<const char*> deviceExtensions;
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	uint32_t gpuCount;
	std::vector<VkPhysicalDevice> gpus;

	// Get the number of devices (GPUs) available.

	auto devices = m_instance->enumeratePhysicalDevices();
	m_gpu = devices[0];

	auto queueProps = m_gpu.getQueueFamilyProperties();

	uint32_t familyIndex = 0;
	for(int i = 0; i < queueProps.size(); i++)
	{
		if ( (queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics) != vk::QueueFlagBits() )
		{
			printf("GOT IT: family %d\n", familyIndex);
			break;
		}
		familyIndex++;
	}

	m_queueFamilyIndex = familyIndex;

	// now we assume that this queue can present or else.
	auto canPresent = m_gpu.getSurfaceSupportKHR(m_queueFamilyIndex, *m_presentSurface);
	assert(canPresent);

	float queue_priorities[1] = { 1.0f };

	vk::DeviceQueueCreateInfo queueInfo;
	queueInfo.queueCount = 1;
	queueInfo.queueFamilyIndex = familyIndex;
	queueInfo.pQueuePriorities = queue_priorities;

	vk::DeviceCreateInfo deviceInfo;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = deviceExtensions.size();
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = NULL;
	deviceInfo.pEnabledFeatures = NULL;

	m_device = m_gpu.createDevice(deviceInfo);

	m_queue = m_device.getQueue(m_queueFamilyIndex, 0);
	m_memoryProps = m_gpu.getMemoryProperties();
}


void Renderer::CreateCmdBuffer()
{

	vk::CommandPoolCreateInfo cmdPoolInfo;
	cmdPoolInfo.queueFamilyIndex = m_queueFamilyIndex;
	cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	m_cmdBufferPool = m_device.createCommandPoolUnique(cmdPoolInfo);

	vk::CommandBufferAllocateInfo cmd;
	cmd.commandPool = *m_cmdBufferPool;
	cmd.level = vk::CommandBufferLevel::ePrimary;
	cmd.commandBufferCount = 1;

	auto bufs = m_device.allocateCommandBuffersUnique(cmd);
	m_cmdBuffer = std::move(bufs[0]);
}


void Renderer::CreateSwapchain(sf::Window* window)
{
	auto formats = m_gpu.getSurfaceFormatsKHR(*m_presentSurface);

	auto surfaceFormat = formats[0].format;
	m_swapchainFormat = surfaceFormat;

	auto surfaceCaps = m_gpu.getSurfaceCapabilitiesKHR(*m_presentSurface);

	auto modes = m_gpu.getSurfacePresentModesKHR(*m_presentSurface);

	vk::SurfaceTransformFlagBitsKHR preTransform;
	if (surfaceCaps.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
	{
		preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	}
	else
	{
		preTransform = surfaceCaps.currentTransform;
	}

	// Find a supported composite alpha mode - one of these is guaranteed to be set
	vk::CompositeAlphaFlagBitsKHR compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	vk::CompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		vk::CompositeAlphaFlagBitsKHR::ePreMultiplied ,
		vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
		vk::CompositeAlphaFlagBitsKHR::eInherit,
	};

	for (uint32_t i = 0; i < sizeof(compositeAlphaFlags); i++) {
		if (surfaceCaps.supportedCompositeAlpha & compositeAlphaFlags[i]) {
			compositeAlpha = compositeAlphaFlags[i];
			break;
		}
	}

	vk::SwapchainCreateInfoKHR swapchain_ci;
	swapchain_ci.surface = *m_presentSurface;
	swapchain_ci.minImageCount = surfaceCaps.minImageCount;
	swapchain_ci.imageFormat = surfaceFormat;
	swapchain_ci.imageExtent.width = window->getSize().x;
	swapchain_ci.imageExtent.height = window->getSize().y;
	swapchain_ci.preTransform = preTransform;
	swapchain_ci.compositeAlpha = compositeAlpha;
	swapchain_ci.imageArrayLayers = 1;
	swapchain_ci.presentMode = vk::PresentModeKHR::eFifo;
	swapchain_ci.oldSwapchain = vk::SwapchainKHR();
	swapchain_ci.clipped = true;
	swapchain_ci.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	swapchain_ci.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	swapchain_ci.imageSharingMode = vk::SharingMode::eExclusive;
	swapchain_ci.queueFamilyIndexCount = 0;
	swapchain_ci.pQueueFamilyIndices = NULL;

	m_swapchain = m_device.createSwapchainKHRUnique(swapchain_ci);
	auto images = m_device.getSwapchainImagesKHR(*m_swapchain);
	m_swapchainImages = std::move(images);

	m_swapchainImageViews.resize(m_swapchainImages.size());

	int i = 0;
	for (auto& image : m_swapchainImages)
	{
		VkImageViewCreateInfo color_image_view = {};
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_image_view.pNext = NULL;
		color_image_view.flags = 0;
		color_image_view.image = image;
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_image_view.format = (VkFormat) m_swapchainFormat;
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_image_view.subresourceRange.baseMipLevel = 0;
		color_image_view.subresourceRange.levelCount = 1;
		color_image_view.subresourceRange.baseArrayLayer = 0;
		color_image_view.subresourceRange.layerCount = 1;

		auto cool = vkCreateImageView(m_device, &color_image_view, NULL, reinterpret_cast<VkImageView*>(&m_swapchainImageViews[i]) );
		assert(cool == VK_SUCCESS);
		i++;
	}
}




VkBool32 SuperDebug(
	VkDebugReportFlagsEXT                       flags,
	VkDebugReportObjectTypeEXT                  objectType,
	uint64_t                                    object,
	size_t                                      location,
	int32_t                                     messageCode,
	const char*                                 pLayerPrefix,
	const char*                                 pMessage,
	void*                                       pUserData)
{
	const char* level;
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		level = "INFO";
	}
	else if ( flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		level = "WARNING";
	}
	else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		level = "ERROR";
	}
	else
	{
		level = "PERF";
	}

	printf("VULKAN %s: %s\n", level, pMessage);
	return VK_FALSE;
}


void InitDebugReports(VkInstance instance)
{
	VkDebugReportCallbackEXT cb1;
	VkDebugReportCallbackCreateInfoEXT callback1 = {
		VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,    // sType
		NULL,                                                       // pNext
		VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT,
		//VK_DEBUG_REPORT_DEBUG_BIT_EXT,
		SuperDebug,													 // pfnCallback
		NULL                                                        // pUserData
	};

	PFN_vkCreateDebugReportCallbackEXT createDebugReport = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	auto res = createDebugReport(instance, &callback1, nullptr, &cb1);
}


bool GetMemoryTypeIndexFromProps(const VkPhysicalDeviceMemoryProperties& memoryProps, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < memoryProps.memoryTypeCount; i++) {
		if ((typeBits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((memoryProps.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}


void Renderer::CreateDepthBuffer()
{
	vk::ImageCreateInfo image_info;

	image_info.imageType = vk::ImageType::e2D;
	image_info.format = vk::Format::eD24UnormS8Uint;
	image_info.extent.width = m_framebufferWidth;
	image_info.extent.height = m_framebufferHeight;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = vk::SampleCountFlagBits::e1;
	image_info.initialLayout = vk::ImageLayout::eUndefined;
	image_info.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = NULL;
	image_info.sharingMode = vk::SharingMode::eExclusive;
	image_info.tiling = vk::ImageTiling::eOptimal;

	m_depthBuffer = CreateImage(m_framebufferWidth, m_framebufferHeight, vk::Format::eD24UnormS8Uint, vk::ImageTiling::eOptimal, vk::Sampler(), vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);

}

/*
void CreateUniformBuffer(VulkanData& data)
{
	auto Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	auto View = glm::lookAt(
		glm::vec3(-5, 10, -10), // Camera is at (-5,3,-10), in World Space
		glm::vec3(0, 0, 0),    // and looks at the origin
		glm::vec3(0, -1, 0)    // Head is up (set to 0,-1,0 to look upside-down)
	);
	auto Model = glm::mat4(1.0f);
	// Vulkan clip space has inverted Y and half Z.
	auto Clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f);

	auto MVP = Clip * Projection * View * Model;

	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buf_info.size = sizeof(MVP);
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = NULL;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;
	auto res = vkCreateBuffer(data.device, &buf_info, NULL, &data.uniformBuffer.buffer);
	assert(res == VK_SUCCESS);

	data.uniformBuffer.descriptorInfo.buffer = data.uniformBuffer.buffer;
	data.uniformBuffer.descriptorInfo.offset = 0;
	data.uniformBuffer.descriptorInfo.range = sizeof(MVP);

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(data.device, data.uniformBuffer.buffer, &mem_reqs);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.memoryTypeIndex = 0;
	alloc_info.allocationSize = mem_reqs.size;
	data.uniformBuffer.memorySize = mem_reqs.size;

	auto pass = GetMemoryTypeIndexFromProps(data.memoryProps, mem_reqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&alloc_info.memoryTypeIndex);
	assert(pass && "No mappable, coherent memory");

	VK_CHECK( vkAllocateMemory(data.device, &alloc_info, NULL, &data.uniformBuffer.memory) );

	void* cpuMem = nullptr;
	VK_CHECK( vkMapMemory(data.device, data.uniformBuffer.memory, 0, mem_reqs.size, 0, &cpuMem) );
	memcpy(cpuMem, &MVP, sizeof(MVP));

	vkUnmapMemory(data.device, data.uniformBuffer.memory);

	VK_CHECK( vkBindBufferMemory(data.device, data.uniformBuffer.buffer, data.uniformBuffer.memory, 0) );
}

void CreateDescriptors()
{
	VkDescriptorSetLayoutBinding vertex_binding = {};
	vertex_binding.binding = 0;
	vertex_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vertex_binding.descriptorCount = 1;
	vertex_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_binding.pImmutableSamplers = NULL;

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	vkCreateSampler(data.device, &samplerInfo, nullptr, &data.linearClampSampler);


	VkDescriptorSetLayoutBinding fragment_binding = {};
	fragment_binding.binding = 1;
	fragment_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	fragment_binding.descriptorCount = 1;
	fragment_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragment_binding.pImmutableSamplers = nullptr;

#define NUM_DESCRIPTOR_SETS 1

	VkDescriptorSetLayoutBinding bindings[] = { vertex_binding, fragment_binding };

	VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
	descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout.pNext = NULL;
	descriptor_layout.bindingCount = 2;
	descriptor_layout.pBindings = bindings;
	data.descriptorLayouts.resize(NUM_DESCRIPTOR_SETS);
	VK_CHECK( vkCreateDescriptorSetLayout(data.device, &descriptor_layout, NULL, data.descriptorLayouts.data()) );

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount = NUM_DESCRIPTOR_SETS;
	pPipelineLayoutCreateInfo.pSetLayouts = data.descriptorLayouts.data();

	VK_CHECK( vkCreatePipelineLayout(data.device, &pPipelineLayoutCreateInfo, NULL, &data.pipelineLayout) );

	VkDescriptorPoolSize poolSizes[2];
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptor_pool = {};
	descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool.pNext = NULL;
	descriptor_pool.maxSets = 1;
	descriptor_pool.poolSizeCount = 2;
	descriptor_pool.pPoolSizes = poolSizes;

	VK_CHECK( vkCreateDescriptorPool(data.device, &descriptor_pool, NULL, &data.descriptorPool) );

	VkDescriptorSetAllocateInfo alloc_info[1];
	alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info[0].pNext = NULL;
	alloc_info[0].descriptorPool = data.descriptorPool;
	alloc_info[0].descriptorSetCount = NUM_DESCRIPTOR_SETS;
	alloc_info[0].pSetLayouts = data.descriptorLayouts.data();
	data.descriptorSets.resize(NUM_DESCRIPTOR_SETS);
	VK_CHECK( vkAllocateDescriptorSets(data.device, alloc_info, data.descriptorSets.data()) );


	VkWriteDescriptorSet writes[2];
	writes[0] = {};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].pNext = NULL;
	writes[0].dstSet = data.descriptorSets[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo = &data.uniformBuffer.descriptorInfo;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;


	VkDescriptorImageInfo descriptorImageInfo;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorImageInfo.imageView = VK_NULL_HANDLE;
	descriptorImageInfo.sampler = VK_NULL_HANDLE;

	writes[1] = {};
	writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[1].pNext = NULL;
	writes[1].dstSet = data.descriptorSets[0];
	writes[1].descriptorCount = 1;
	writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writes[1].pImageInfo = nullptr;// &descriptorImageInfo;
	writes[1].dstArrayElement = 0;
	writes[1].dstBinding = 1;

	// vkUpdateDescriptorSets(data.device, 2, writes, 0, NULL);

}
*/

void Renderer::CreateRenderPass()
{
	// woowowoow we're gonna leave this in the c way because omg.

	VkAttachmentDescription attachments[2];
	attachments[0].format = (VkFormat)m_swapchainFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachments[0].flags = 0;

	attachments[1].format = VK_FORMAT_D24_UNORM_S8_UINT;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].flags = 0;

	VkAttachmentReference color_reference = {};
	color_reference.attachment = 0;
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_reference = {};
	depth_reference.attachment = 1;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_reference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = &depth_reference;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo rp_info = {};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = 2;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 0;
	rp_info.pDependencies = NULL;
	auto res =  vkCreateRenderPass(m_device, &rp_info, NULL, reinterpret_cast<VkRenderPass*>(&m_renderPass));
	assert(res == VK_SUCCESS);

	VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
	imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	imageAcquiredSemaphoreCreateInfo.pNext = NULL;
	imageAcquiredSemaphoreCreateInfo.flags = 0;

	VK_CHECK(vkCreateSemaphore(m_device, &imageAcquiredSemaphoreCreateInfo, NULL, (VkSemaphore*)&m_imageAcquireSemaphore));

	VkFenceCreateInfo fenceInfo;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = 0;
	vkCreateFence(m_device, &fenceInfo, NULL, (VkFence*)&m_frameFence);

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	vkCreateSampler(m_device, &samplerInfo, nullptr, (VkSampler*)&m_linearClampSampler);
}


void Renderer::CreateFrameBuffer()
{
	VkImageView attachments[2];
	attachments[1] = *m_depthBuffer.view;

	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = NULL;
	fb_info.renderPass = m_renderPass;
	fb_info.attachmentCount = 2;
	fb_info.pAttachments = attachments;
	fb_info.width = m_framebufferWidth;
	fb_info.height = m_framebufferHeight;
	fb_info.layers = 1;

	attachments[0] = m_swapchainImageViews[0];
	vkCreateFramebuffer(m_device, &fb_info, NULL, (VkFramebuffer*)&m_framebuffers[0]);

	attachments[0] = m_swapchainImageViews[1];
	vkCreateFramebuffer(m_device, &fb_info, NULL, (VkFramebuffer*)&m_framebuffers[1]);
}

#include "cube_data.h"

template<typename T>
vk::MemoryRequirements GetMemoryRequirements(vk::Device device, T& obj)
{
	if constexpr(std::is_same<T, GpuBuffer>().value)
	{
		return device.getBufferMemoryRequirements(*obj.buffer);
	}
	else if constexpr(std::is_same<T, GpuImage>().value)
	{
		return device.getImageMemoryRequirements(*obj.image);
	}
	else
	{
		assert(false && "Invalid GPU allocation type" );
	}
}

template<typename T>
void Renderer::AllocateGpuMemory(T& obj, vk::MemoryPropertyFlags memoryFlags)
{
	vk::MemoryRequirements reqs = GetMemoryRequirements(m_device, obj);

	vk::MemoryAllocateInfo alloc_info;
	alloc_info.memoryTypeIndex = 0;
	alloc_info.allocationSize = reqs.size;

	auto pass = GetMemoryTypeIndexFromProps(m_memoryProps, reqs.memoryTypeBits, (VkFlags)memoryFlags, &alloc_info.memoryTypeIndex);
	assert(pass && "No memory of requested type.");

	obj.memory = m_device.allocateMemoryUnique(alloc_info);
	obj.memorySize = reqs.size;
}


/*
void CreateVertexBuffer(Renderer& renderer)
{

	auto vertexBuffer =  renderer.CreateBuffer()


	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buf_info.size = sizeof(g_vb_texture_Data);
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = NULL;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;
	
	VK_CHECK( vkCreateBuffer(renderer.device, &buf_info, NULL, &data.vertexBuffer.buffer) );

	
	AllocateGpuMemory(data.device, data.vertexBuffer, NiftyFlags::CpuMappableMemory);

	uint8_t *pData;
	vkMapMemory(data.device, data.vertexBuffer.memory, 0, data.vertexBuffer.memorySize, 0, (void **)&pData);

	memcpy(pData, g_vb_texture_Data, sizeof(g_vb_texture_Data));

	vkUnmapMemory(data.device, data.vertexBuffer.memory);

	vkBindBufferMemory(data.device, data.vertexBuffer.buffer, data.vertexBuffer.memory, 0);

	data.vertexBinding.binding = 0;
	data.vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	data.vertexBinding.stride = sizeof(g_vb_texture_Data[0]);

	data.vertexAttributes[0].binding = 0;
	data.vertexAttributes[0].location = 0;
	data.vertexAttributes[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	data.vertexAttributes[0].offset = 0;

	data.vertexAttributes[1].binding = 0;
	data.vertexAttributes[1].location = 1;
	data.vertexAttributes[1].format = VK_FORMAT_R32G32_SFLOAT;
	data.vertexAttributes[1].offset = 16;
}


void Renderer::CreatePipeline()
{
	VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext = NULL;
	dynamicState.pDynamicStates = dynamicStateEnables;
	dynamicState.dynamicStateCount = 0;

	VkPipelineVertexInputStateCreateInfo vi;
	vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vi.pNext = NULL;
	vi.flags = 0;
	vi.vertexBindingDescriptionCount = 1;
	vi.pVertexBindingDescriptions = &m_vertexBinding;
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

	VK_CHECK( vkCreateGraphicsPipelines(m_device, NULL, 1, &pipelineInfo, NULL, &m_pipeline) );
}
*/


vk::CommandBuffer Renderer::BeginRenderPass()
{
	// Get the index of the next available swapchain image:
	VK_CHECK(vkAcquireNextImageKHR(m_device, *m_swapchain, UINT64_MAX, m_imageAcquireSemaphore, VK_NULL_HANDLE, &m_currentSwapchainIndex));

	VkCommandBufferBeginInfo cmd_buf_info = {};
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buf_info.pNext = NULL;
	cmd_buf_info.flags = 0;
	cmd_buf_info.pInheritanceInfo = NULL;

	VkClearValue clear_values[2];
	clear_values[0].color.float32[0] = 0.2f;
	clear_values[0].color.float32[1] = 0.2f;
	clear_values[0].color.float32[2] = 0.2f;
	clear_values[0].color.float32[3] = 0.2f;
	clear_values[1].depthStencil.depth = 1.0f;
	clear_values[1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo rp_begin;
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = NULL;
	rp_begin.renderPass = m_renderPass;
	rp_begin.framebuffer = m_framebuffers[m_currentSwapchainIndex];
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = m_framebufferWidth;
	rp_begin.renderArea.extent.height = m_framebufferHeight;
	rp_begin.clearValueCount = 2;
	rp_begin.pClearValues = clear_values;


	VK_CHECK(vkBeginCommandBuffer(*m_cmdBuffer, &cmd_buf_info));
	vkCmdBeginRenderPass(*m_cmdBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

	vkResetFences(m_device, 1, (VkFence*)&m_frameFence);

	return *m_cmdBuffer;
}


void Renderer::EndRenderPass(vk::CommandBuffer cmdBuffer)
{
	vkCmdEndRenderPass(cmdBuffer);
	VK_CHECK(vkEndCommandBuffer(cmdBuffer));
}

void Renderer::SubmitCommandBuffers(vk::CommandBuffer* cmdBuffers, uint32_t commandBufferCount, vk::Fence fence)
{
	vk::PipelineStageFlags pipe_stage_flags = vk::PipelineStageFlagBits::eBottomOfPipe;
	vk::SubmitInfo submit_info[1];
	submit_info[0].waitSemaphoreCount = 1;
	submit_info[0].pWaitSemaphores = &m_imageAcquireSemaphore;
	submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
	submit_info[0].commandBufferCount = commandBufferCount;
	submit_info[0].pCommandBuffers = cmdBuffers;
	submit_info[0].signalSemaphoreCount = 0;
	submit_info[0].pSignalSemaphores = NULL;

	m_queue.submit(1, submit_info, fence);
}

bool Renderer::WaitForFence(vk::Fence fence, uint64_t timeoutNanos)
{
	auto res = m_device.waitForFences(1, &fence, true, timeoutNanos);
	return res == vk::Result::eTimeout;
}

void Renderer::SetViewportAndScissor(vk::CommandBuffer cmdBuffer, sf::Vector2u size, sf::Vector2u offset)
{
	m_viewport.width = static_cast<float>(size.x);
	m_viewport.height = static_cast<float>(size.y);
	m_viewport.minDepth = 0.0f;
	m_viewport.maxDepth = 1.0f;
	m_viewport.x = static_cast<float>(offset.x);
	m_viewport.y = static_cast<float>(offset.y);

	cmdBuffer.setViewport(0, 1, &m_viewport);

	m_scissor.extent.width = size.x;
	m_scissor.extent.height = size.y;
	m_scissor.offset.x = offset.x;
	m_scissor.offset.y = offset.y;
	cmdBuffer.setScissor(0, 1, &m_scissor);
}

void Renderer::Present()
{
	vk::PresentInfoKHR present;
	present.swapchainCount = 1;
	present.pSwapchains = &m_swapchain.get();
	present.pImageIndices = &m_currentSwapchainIndex;
	present.pWaitSemaphores = NULL;
	present.waitSemaphoreCount = 0;
	present.pResults = NULL;
	m_queue.presentKHR(present);
}


/*
void Cube(VulkanData& data)
{

	// Get the index of the next available swapchain image:

	VkCommandBufferBeginInfo cmd_buf_info = {};
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buf_info.pNext = NULL;
	cmd_buf_info.flags = 0;
	cmd_buf_info.pInheritanceInfo = NULL;

	VkClearValue clear_values[2];
	clear_values[0].color.float32[0] = 0.2f;
	clear_values[0].color.float32[1] = 0.2f;
	clear_values[0].color.float32[2] = 0.2f;
	clear_values[0].color.float32[3] = 0.2f;
	clear_values[1].depthStencil.depth = 1.0f;
	clear_values[1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo rp_begin;
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = NULL;
	rp_begin.renderPass = data.renderPass;
	rp_begin.framebuffer = data.framebuffers[data.currentSwapchainIndex];
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = data.framebufferWidth;
	rp_begin.renderArea.extent.height = data.framebufferHeight;
	rp_begin.clearValueCount = 2;
	rp_begin.pClearValues = clear_values;
	
	VK_CHECK(vkBeginCommandBuffer(data.cmdBuffer, &cmd_buf_info));

	vkCmdBeginRenderPass(data.cmdBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(data.cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipeline);
	vkCmdBindDescriptorSets(data.cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipelineLayout, 0, 1, data.descriptorSets.data(), 0, NULL);
	
	const VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(data.cmdBuffer, 0, 1, &data.vertexBuffer.buffer, offsets);

	data.viewport.width = (float)data.framebufferWidth;
	data.viewport.height = (float)data.framebufferHeight;
	data.viewport.minDepth = (float)0.0f;
	data.viewport.maxDepth = (float)1.0f;
	data.viewport.x = 0;
	data.viewport.y = 0;
	vkCmdSetViewport(data.cmdBuffer, 0, 1, &data.viewport);

	data.scissor.extent.width = data.framebufferWidth;
	data.scissor.extent.height = data.framebufferHeight;
	data.scissor.offset.x = 0;
	data.scissor.offset.y = 0;
	vkCmdSetScissor(data.cmdBuffer, 0, 1, &data.scissor);

	vkCmdDraw(data.cmdBuffer, 12 * 3, 1, 0, 0);
	vkCmdEndRenderPass(data.cmdBuffer);

	VK_CHECK( vkEndCommandBuffer(data.cmdBuffer) );


	const VkCommandBuffer cmd_bufs[] = { data.cmdBuffer };
	VkPipelineStageFlags pipe_stage_flags =
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo submit_info[1] = {};
	submit_info[0].pNext = NULL;
	submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info[0].waitSemaphoreCount = 1;
	submit_info[0].pWaitSemaphores = &data.imageAcquireSemaphore;
	submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
	submit_info[0].commandBufferCount = 1;
	submit_info[0].pCommandBuffers = cmd_bufs;
	submit_info[0].signalSemaphoreCount = 0;
	submit_info[0].pSignalSemaphores = NULL;
	VK_CHECK( vkQueueSubmit(data.queue, 1, submit_info, data.frameFence) );

	VkResult res;
	do {
		 res = vkWaitForFences(data.device, 1, &data.frameFence, VK_TRUE, 100000000);
	} while (res == VK_TIMEOUT);

	VK_CHECK(res);

	VkPresentInfoKHR present;
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pNext = NULL;
	present.swapchainCount = 1;
	present.pSwapchains = &data.swapchain;
	present.pImageIndices = &data.currentSwapchainIndex;
	present.pWaitSemaphores = NULL;
	present.waitSemaphoreCount = 0;
	present.pResults = NULL;
	VK_CHECK( vkQueuePresentKHR(data.queue, &present) );

}

static VulkanData s_vulkanData;

void InitVulkan(sf::Window* window, unsigned char* texture, int x, int y)
{
	s_vulkanData.framebufferWidth = window->getSize().x;
	s_vulkanData.framebufferHeight = window->getSize().y;

	CreateVulkanInstance(s_vulkanData);



	InitDebugReports(s_vulkanData.instance);

	CreateSurface(window, s_vulkanData);

	CreateVulkanDevice(s_vulkanData);
	s_memoryProps = s_vulkanData.memoryProps;

	CreateVulkanCmdBuffer(s_vulkanData);

	CreateSwapchain(s_vulkanData, window);

	CreateDepthBuffer(s_vulkanData);

	CreateUniformBuffer(s_vulkanData);
	
	CreateDescriptors(s_vulkanData);

	CreateRenderPass(s_vulkanData);

	CreateShaders(s_vulkanData);

	CreateFrameBuffer(s_vulkanData);

	CreateVertexBuffer(s_vulkanData);

	CreatePipeline(s_vulkanData);

	UploadTexture(texture, x, y);

	Cube(s_vulkanData);
}
*/

glm::mat4x4 GetMVP()
{
	static sf::Clock rotator;
	static float speed = 1.0f;

	auto Projection = glm::perspective(glm::radians(45.0f), 1000.0f/600.0f, 0.1f, 100.0f);
	auto View = glm::lookAt(
		glm::vec3(0, 0, -10), // Camera is at (-5,3,-10), in World Space
		glm::vec3(0, 0, 0),    // and looks at the origin
		glm::vec3(0, 1, 0)    // Head is up (set to 0,-1,0 to look upside-down)
	);
	auto model = glm::mat4(1.0f);
	auto axis = glm::vec3(1.0f, 1.0f, 1.0f);
	float angle = speed * rotator.getElapsedTime().asSeconds();
	model = glm::rotate(model, angle , axis);
	// Vulkan clip space has inverted Y and half Z.
	auto Clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f);

	auto MVP = Clip * Projection * View * model;
	return MVP;
}

/*
void PerFrame(VulkanData& data, sf::Window* window)
{
	auto cmdBuffer = BeginRenderPass(data);
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipeline);
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipelineLayout, 0, 1, data.descriptorSets.data(), 0, NULL);
	SetViewportAndScissor(data, cmdBuffer, window->getSize(), { 0,0 });

	auto mvp = GetMVP();

	void* cpuMem;
	vkMapMemory(data.device, data.uniformBuffer.memory, 0, data.uniformBuffer.memorySize, 0, &cpuMem);
	memcpy(cpuMem, &mvp, sizeof(mvp));
	vkUnmapMemory(data.device, data.uniformBuffer.memory);

	const VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &data.vertexBuffer.buffer, offsets);
	vkCmdDraw(data.cmdBuffer, 12 * 3, 1, 0, 0);

	EndRenderPass(cmdBuffer);

	SubmitCommandBuffers(data, &cmdBuffer, 1, data.frameFence);

	auto nanos = sf::seconds(1.0f).asNanoseconds();

	if (WaitForFence(data, data.frameFence, nanos))
	{
		printf("Timed out waiting for render.\n");
	}

	Present(data);
}

void RenderFrame(sf::Window* window)
{
	PerFrame(s_vulkanData, window);
}
*/


GpuBuffer Renderer::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlags, NiftyFlags flags)
{
	vk::MemoryPropertyFlagBits flagBits = (vk::MemoryPropertyFlagBits)flags;
	return CreateBuffer(size, usageFlags, flagBits);
}

GpuBuffer Renderer::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryProperties)
{
	GpuBuffer buffer;

	vk::BufferCreateInfo buf_info = {};
	buf_info.usage = usageFlags;
	buf_info.size = size;
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = NULL;
	buf_info.sharingMode = vk::SharingMode::eExclusive;
	buffer.buffer = m_device.createBufferUnique(buf_info);

	AllocateGpuMemory(buffer, memoryProperties);
	VK_CHECK(vkBindBufferMemory(m_device, *buffer.buffer, *buffer.memory, 0));

	vk::DescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = *buffer.buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = size;
	buffer.descriptorInfo = descriptorBufferInfo;

	return buffer;
}


vk::UniqueImageView CreateImageView(vk::Device device, vk::Image image, vk::Format format)
{
	vk::ImageViewCreateInfo color_image_view = {};
	color_image_view.image = image;
	color_image_view.viewType = vk::ImageViewType::e2D;
	color_image_view.format = format;
	color_image_view.components.r = (vk::ComponentSwizzle) VK_COMPONENT_SWIZZLE_R;
	color_image_view.components.g = (vk::ComponentSwizzle) VK_COMPONENT_SWIZZLE_G;
	color_image_view.components.b = (vk::ComponentSwizzle) VK_COMPONENT_SWIZZLE_B;
	color_image_view.components.a = (vk::ComponentSwizzle) VK_COMPONENT_SWIZZLE_A;
	color_image_view.subresourceRange.aspectMask = (format == vk::Format::eD24UnormS8Uint) ? vk::ImageAspectFlagBits::eDepth :  vk::ImageAspectFlagBits::eColor;
	color_image_view.subresourceRange.baseMipLevel = 0;
	color_image_view.subresourceRange.levelCount = 1;
	color_image_view.subresourceRange.baseArrayLayer = 0;
	color_image_view.subresourceRange.layerCount = 1;

	return device.createImageViewUnique(color_image_view);
}


GpuImage Renderer::CreateImage(uint32_t x, uint32_t y, vk::Format format, vk::ImageTiling tiling, vk::Sampler sampler, vk::ImageUsageFlags usageFlags, vk::MemoryPropertyFlags memoryProperties)
{
	GpuImage image;
	vk::ImageCreateInfo image_info;

	image_info.imageType = vk::ImageType::e2D;
	image_info.format = format;
	image_info.extent.width = x;
	image_info.extent.height = y;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = vk::SampleCountFlagBits::e1;
	image_info.initialLayout = vk::ImageLayout::eUndefined;
	image_info.usage = usageFlags;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = NULL;
	image_info.sharingMode = vk::SharingMode::eExclusive;
	image_info.tiling = tiling;

	image.image = m_device.createImageUnique(image_info);

	AllocateGpuMemory(image, memoryProperties);
	VK_CHECK( vkBindImageMemory(m_device, *image.image, *image.memory, 0) );

	image.view = CreateImageView(m_device, *image.image, format);

	VkDescriptorImageInfo descriptorImageInfo = {};
	descriptorImageInfo.sampler = sampler;
	descriptorImageInfo.imageView = *image.view;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	image.descriptorInfo = descriptorImageInfo;

	return image;
}

void Renderer::CopyBufferToImage(vk::CommandBuffer cmdBuffer, vk::Buffer srcBuffer, vk::Image dstImage, uint32_t x, uint32_t y)
{
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		x,
		y,
		1
	};

	vkCmdCopyBufferToImage(
		cmdBuffer,
		srcBuffer,
		dstImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
}


void Renderer::TransitionImage(vk::CommandBuffer cmdBuffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout )
{
	vk::ImageMemoryBarrier barrier = {};
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage, destinationStage;

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		barrier.srcAccessMask = vk::AccessFlagBits();
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		assert(false && "Unsupported Transition!");
	}

	vkCmdPipelineBarrier(
		cmdBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, (VkImageMemoryBarrier*)&barrier
	);
}



void Renderer::Init(sf::Window* window)
{
	m_framebufferWidth = window->getSize().x;
	m_framebufferHeight = window->getSize().y;

	CreateVulkanInstance();
	InitDebugReports(*m_instance);
	CreateSurface(window);
	CreateVulkanDevice();
	CreateCmdBuffer();
	CreateSwapchain(window);
	CreateDepthBuffer();
	CreateRenderPass();
	CreateFrameBuffer();



}


GpuImage Renderer::UploadTexture(std::byte* texture, int x, int y)
{
	size_t imageSize = x * y * sizeof(char);

	auto stagingBuffer = CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, NiftyFlags::CpuMappableMemory);

	auto cpuMemory = stagingBuffer.Map(m_device);
	memcpy(cpuMemory, texture, imageSize);
	stagingBuffer.Unmap(m_device);

	auto gpuTexture = CreateImage(x, y, vk::Format::eR8Unorm, vk::ImageTiling::eOptimal, m_linearClampSampler, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);

	/*
	VkWriteDescriptorSet writes[2];
	writes[0] = {};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].pNext = NULL;
	writes[0].dstSet = data.descriptorSets[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo = &data.uniformBuffer.descriptorInfo;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;

	writes[1] = {};
	writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[1].pNext = NULL;
	writes[1].dstSet = data.descriptorSets[0];
	writes[1].descriptorCount = 1;
	writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writes[1].pImageInfo = &gpuTexture.descriptorInfo;
	writes[1].dstArrayElement = 0;
	writes[1].dstBinding = 1;

	vkUpdateDescriptorSets(data.device, 1, writes, 0, NULL);
	*/

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	vkBeginCommandBuffer(*m_cmdBuffer, &beginInfo);
	TransitionImage(*m_cmdBuffer, *gpuTexture.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	CopyBufferToImage(*m_cmdBuffer, *stagingBuffer.buffer, *gpuTexture.image, x, y);
	TransitionImage(*m_cmdBuffer, *gpuTexture.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	vkEndCommandBuffer(*m_cmdBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = (VkCommandBuffer*)&m_cmdBuffer.get();

	vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_queue);

	return gpuTexture;
}

Pipeline::Pipeline(vk::Device device)
{
	m_device = device;
}

vk::DescriptorSetLayoutBinding* Pipeline::HasExistingBinding(int bindingPoint)
{
	for (auto& binding : m_bindings)
	{
		if (bindingPoint == binding.binding)
		{
			&binding;
		}
	}

	return nullptr;
}

void Pipeline::AddVertexBufferBinding(int bindingPoint, int size, vk::VertexInputRate inputRate)
{
	m_vertexBufferBindings.push_back(vk::VertexInputBindingDescription(bindingPoint, size, inputRate));
}

void Pipeline::AddVertexBufferAttribute(int bindingPoint, int indexInBinding, vk::Format format, int offset)
{
	m_vertexAttributes.push_back(vk::VertexInputAttributeDescription(indexInBinding, bindingPoint, format, offset));
}

void Pipeline::AddVertexBinding(int bindingPoint, int count, vk::DescriptorType type)
{
	auto maybeBinding = HasExistingBinding(bindingPoint);
	if (maybeBinding)
	{
		auto& binding = *maybeBinding;
		assert(binding.descriptorType == type);
		assert(binding.descriptorCount == count);
		maybeBinding->stageFlags |= vk::ShaderStageFlagBits::eVertex;
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
}

void Pipeline::AddFragmentBinding(int bindingPoint, int count, vk::DescriptorType type)
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
}

void Pipeline::CreateDescriptors(int maxSets)
{
	vk::DescriptorSetLayoutCreateInfo descriptor_layout;
	descriptor_layout.bindingCount = m_bindings.size();
	descriptor_layout.pBindings = m_bindings.data();
	m_descriptorLayout = m_device.createDescriptorSetLayoutUnique(descriptor_layout);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_descriptorLayout.get();

	m_pipelineLayout = m_device.createPipelineLayoutUnique(pipelineLayoutInfo);

	std::vector<vk::DescriptorPoolSize> poolSizes;
	for (auto& binding : m_bindings)
	{
		vk::DescriptorPoolSize size{ binding.descriptorType, binding.descriptorCount * maxSets };
		poolSizes.push_back(size);
	}

	vk::DescriptorPoolCreateInfo descriptor_pool;
	descriptor_pool.maxSets = maxSets;
	descriptor_pool.poolSizeCount = poolSizes.size();
	descriptor_pool.pPoolSizes = poolSizes.data();

	m_descriptorPool = m_device.createDescriptorPoolUnique(descriptor_pool);
}

void Pipeline::CreateVertexShader(const char* shaderText, const char* shaderName)
{
	auto spv = CompileShaderString(shaderText, shaderc_glsl_vertex_shader, shaderName);
	vk::ShaderModuleCreateInfo info;
	info.pCode = spv.data();
	info.codeSize = spv.size() * sizeof(uint32_t);

	m_vertexModule = m_device.createShaderModuleUnique(info);

	m_vertexStage.module = m_vertexModule.get();
	m_vertexStage.pName = "main";
	m_vertexStage.stage = vk::ShaderStageFlagBits::eVertex;
}

void Pipeline::CreateFragmentShader(const char* shaderText, const char* shaderName)
{
	auto spv = CompileShaderString(shaderText, shaderc_glsl_fragment_shader, shaderName);
	vk::ShaderModuleCreateInfo info;
	info.pCode = spv.data();
	info.codeSize = spv.size() * sizeof(uint32_t);

	m_fragmentModule = m_device.createShaderModuleUnique(info);

	m_fragmentStage.module = m_fragmentModule.get();
	m_fragmentStage.pName = "main";
	m_fragmentStage.stage = vk::ShaderStageFlagBits::eFragment;
}

void Pipeline::CreatePipeline(vk::RenderPass renderPass, int subpass /*= 0*/)
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
}

std::vector<vk::UniqueDescriptorSet> Pipeline::AllocateDescriptorSets(int count /*= 1*/)
{
	vk::DescriptorSetAllocateInfo alloc_info;
	alloc_info.descriptorPool = m_descriptorPool.get();
	alloc_info.descriptorSetCount = count;
	vk::DescriptorSetLayout layouts[] = { m_descriptorLayout.get(), m_descriptorLayout.get() };
	alloc_info.pSetLayouts = layouts;
	return m_device.allocateDescriptorSetsUnique(alloc_info);
}
