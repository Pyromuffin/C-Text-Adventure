
#include <cstdio>

#ifdef _WIN32
	#include <Windows.h>
	#ifndef VK_USE_PLATFORM_WIN32_KHR
		#define VK_USE_PLATFORM_WIN32_KHR
	#endif
#endif

#include <SFML/Window.hpp>
#include <vulkan/vulkan.hpp>
#include "ShaderCompiler.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>




void init_instance_extension_names( std::vector<const char*>& extensionNames ) {
	extensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
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

struct GpuImage
{
	VkImage image;
	VkImageView view;
	VkDeviceMemory mem;
};

struct GpuBuffer
{
	VkBuffer buffer;
	VkDeviceMemory mem;
	VkDescriptorBufferInfo descriptorInfo;
};

struct VulkanData
{
	VkInstance instance;
	VkPhysicalDevice gpu;
	VkDevice device;
	VkPhysicalDeviceMemoryProperties memoryProps;

	uint32_t queueFamilyIndex;

	VkCommandPool cmdBufferPool;
	VkCommandBuffer cmdBuffer;

	VkSurfaceKHR presentSurface;
	VkFormat swapchainFormat;
	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	GpuImage depthBuffer;
	GpuBuffer uniformBuffer;

	std::vector<VkDescriptorSetLayout> descriptorLayouts;
	std::vector<VkDescriptorSet> descriptorSets;
	VkPipelineLayout pipelineLayout;
	VkDescriptorPool descriptorPool;

	VkRenderPass renderPass;

	VkPipelineShaderStageCreateInfo vertexStage;
	VkPipelineShaderStageCreateInfo fragmentStage;
};


void CreateSurface(sf::Window* window, VulkanData& data)
{
VkResult res;

#ifdef _WIN32
	auto handle = window->getSystemHandle();
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(handle, GWLP_HINSTANCE);

	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.hinstance = hInstance;
	createInfo.hwnd = handle;
	res = vkCreateWin32SurfaceKHR(data.instance, &createInfo, NULL, &data.presentSurface);
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



void CreateVulkanInstance(VulkanData &data)
{
	std::vector<const char*> instanceExtensions;
	init_instance_extension_names(instanceExtensions);


	VkInstanceCreateInfo info;
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.enabledLayerCount = 0;
	info.ppEnabledLayerNames = nullptr;
	info.enabledExtensionCount = instanceExtensions.size();
	info.ppEnabledExtensionNames = instanceExtensions.data();

	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "C Text Adventure";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	info.pApplicationInfo = &appInfo;

	VkInstance instance;
	auto res = vkCreateInstance(&info, nullptr, &instance);

	if (res)
	{
		printf("Vulkan instance creation error\n");
	}

	data.instance = instance;
}


void CreateVulkanDevice(VulkanData& data)
{
	std::vector<const char*> deviceExtensions;
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	uint32_t gpuCount;
	std::vector<VkPhysicalDevice> gpus;

	// Get the number of devices (GPUs) available.
	auto res = vkEnumeratePhysicalDevices(data.instance, &gpuCount, NULL);
	// Allocate space and get the list of devices.
	gpus.resize(gpuCount);
	res = vkEnumeratePhysicalDevices(data.instance, &gpuCount, gpus.data());

	if (res)
	{
		printf("Vulkan enumerate error\n");
	}

	data.gpu = gpus[0];

	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueProps;
	queueProps.resize(queueFamilyCount);

	vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queueFamilyCount, queueProps.data());

	uint32_t familyIndex = 0;
	for (auto fam : queueProps)
	{
		if (fam.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT != 0)
		{
			printf("GOT IT: fam %d\n", familyIndex);
			break;
		}
		familyIndex++;
	}

	data.queueFamilyIndex = familyIndex;

	// now we assume that this queue can present or else.
	VkBool32 canPresent;
	vkGetPhysicalDeviceSurfaceSupportKHR(gpus[0], familyIndex, data.presentSurface, &canPresent);
	assert(canPresent);

	float queue_priorities[1] = { 0.0f };

	VkDeviceQueueCreateInfo queueInfo;
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pNext = nullptr;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = queue_priorities;

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = NULL;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = deviceExtensions.size();
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = NULL;
	deviceInfo.pEnabledFeatures = NULL;

	VkDevice device;
	res = vkCreateDevice(gpus[0], &deviceInfo, NULL, &device);
	assert(res == VK_SUCCESS);

	data.device = device;

	vkGetPhysicalDeviceMemoryProperties(data.gpu, &data.memoryProps);
}






void CreateVulkanCmdBuffer(VulkanData& data)
{

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.pNext = NULL;
	cmdPoolInfo.queueFamilyIndex = data.queueFamilyIndex;
	cmdPoolInfo.flags = 0;


	VkCommandPool cmdPool;
	auto res = vkCreateCommandPool(data.device, &cmdPoolInfo, NULL, &cmdPool);

	data.cmdBufferPool = cmdPool;

	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = NULL;
	cmd.commandPool = cmdPool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	VkCommandBuffer cmdBuffer;

	res = vkAllocateCommandBuffers(data.device, &cmd, &cmdBuffer);
	assert(res == VK_SUCCESS);

	data.cmdBuffer = cmdBuffer;
}


void CreateSwapchain(VulkanData& data, sf::Window* window)
{
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(data.gpu, data.presentSurface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats;
	formats.resize(formatCount);

	vkGetPhysicalDeviceSurfaceFormatsKHR(data.gpu, data.presentSurface, &formatCount, formats.data());

	auto surfaceFormat = formats[0].format;
	data.swapchainFormat = surfaceFormat;

	VkSurfaceCapabilitiesKHR surfaceCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(data.gpu, data.presentSurface, &surfaceCaps);

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(data.gpu, data.presentSurface, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(data.gpu, data.presentSurface, &presentModeCount, presentModes.data());

	VkSurfaceTransformFlagBitsKHR preTransform;
	if (surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfaceCaps.currentTransform;
	}

	// Find a supported composite alpha mode - one of these is guaranteed to be set
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	for (uint32_t i = 0; i < sizeof(compositeAlphaFlags); i++) {
		if (surfaceCaps.supportedCompositeAlpha & compositeAlphaFlags[i]) {
			compositeAlpha = compositeAlphaFlags[i];
			break;
		}
	}

	VkSwapchainCreateInfoKHR swapchain_ci = {};
	swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_ci.pNext = NULL;
	swapchain_ci.surface = data.presentSurface;
	swapchain_ci.minImageCount = surfaceCaps.minImageCount;
	swapchain_ci.imageFormat = surfaceFormat;
	swapchain_ci.imageExtent.width = window->getSize().x;
	swapchain_ci.imageExtent.height = window->getSize().y;
	swapchain_ci.preTransform = preTransform;
	swapchain_ci.compositeAlpha = compositeAlpha;
	swapchain_ci.imageArrayLayers = 1;
	swapchain_ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
	swapchain_ci.clipped = true;
	swapchain_ci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_ci.queueFamilyIndexCount = 0;
	swapchain_ci.pQueueFamilyIndices = NULL;


	VkSwapchainKHR swapchain;
	auto res = vkCreateSwapchainKHR(data.device, &swapchain_ci, NULL, &swapchain);
	assert(res == VK_SUCCESS);

	data.swapchain = swapchain;

	uint32_t swapchainImageCount;
	res = vkGetSwapchainImagesKHR(data.device, data.swapchain, &swapchainImageCount, NULL);
	assert(res == VK_SUCCESS);

	data.swapchainImages.resize(swapchainImageCount);

	res = vkGetSwapchainImagesKHR(data.device, swapchain, &swapchainImageCount, data.swapchainImages.data());
	assert(res == VK_SUCCESS);

	data.swapchainImageViews.resize(swapchainImageCount);
	int i = 0;
	for (auto& image : data.swapchainImages)
	{
		VkImageViewCreateInfo color_image_view = {};
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_image_view.pNext = NULL;
		color_image_view.flags = 0;
		color_image_view.image = image;
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_image_view.format = data.swapchainFormat;
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_image_view.subresourceRange.baseMipLevel = 0;
		color_image_view.subresourceRange.levelCount = 1;
		color_image_view.subresourceRange.baseArrayLayer = 0;
		color_image_view.subresourceRange.layerCount = 1;

		res = vkCreateImageView(data.device, &color_image_view, NULL, &data.swapchainImageViews[i]);
		assert(res == VK_SUCCESS);
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

	printf("VULKAN DEBUG: %s\n", pMessage);
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

#define VK_CHECK( func ) \
assert( func == VK_SUCCESS )

void CreateDepthBuffer(VulkanData& data, int width, int height)
{
	VkImageCreateInfo image_info;

	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = NULL;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = VK_FORMAT_D16_UNORM;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = NULL;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.flags = 0;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;


	vkCreateImage(data.device, &image_info, NULL, &data.depthBuffer.image);

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = NULL;
	view_info.image = data.depthBuffer.image;
	view_info.format = VK_FORMAT_D16_UNORM;
	view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.flags = 0;

	VkMemoryRequirements memoryReq;
	vkGetImageMemoryRequirements(data.device, data.depthBuffer.image, &memoryReq);


	uint32_t typeIndex;
	auto res = GetMemoryTypeIndexFromProps(data.memoryProps, memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &typeIndex);
	assert(res);

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = memoryReq.size;
	mem_alloc.memoryTypeIndex = typeIndex;

	
	VK_CHECK( vkAllocateMemory(data.device, &mem_alloc, nullptr, &data.depthBuffer.mem) );
	VK_CHECK( vkBindImageMemory(data.device, data.depthBuffer.image, data.depthBuffer.mem, 0) );
	VK_CHECK( vkCreateImageView(data.device, &view_info, nullptr, &data.depthBuffer.view) );
	

}

void CreateUniformBuffer(VulkanData& data)
{
	auto Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	auto View = glm::lookAt(
		glm::vec3(-5, 3, -10), // Camera is at (-5,3,-10), in World Space
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
	VK_CHECK( vkCreateBuffer(data.device, &buf_info, NULL, &data.uniformBuffer.buffer) );

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


	auto pass = GetMemoryTypeIndexFromProps(data.memoryProps, mem_reqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&alloc_info.memoryTypeIndex);
	assert(pass && "No mappable, coherent memory");

	VK_CHECK( vkAllocateMemory(data.device, &alloc_info, NULL, &data.uniformBuffer.mem) );

	void* cpuMem;
	VK_CHECK( vkMapMemory(data.device, data.uniformBuffer.mem, 0, mem_reqs.size, 0, &cpuMem) );
	memcpy(cpuMem, &MVP, sizeof(MVP));

	vkUnmapMemory(data.device, data.uniformBuffer.mem);

	VK_CHECK( vkBindBufferMemory(data.device, data.uniformBuffer.buffer, data.uniformBuffer.mem, 0) );
}

void CreateDescriptors(VulkanData& data)
{
	VkDescriptorSetLayoutBinding layout_binding = {};
	layout_binding.binding = 0;
	layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_binding.descriptorCount = 1;
	layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layout_binding.pImmutableSamplers = NULL;

#define NUM_DESCRIPTOR_SETS 1
	VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
	descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout.pNext = NULL;
	descriptor_layout.bindingCount = 1;
	descriptor_layout.pBindings = &layout_binding;
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

	VkDescriptorPoolSize type_count[1];
	type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	type_count[0].descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptor_pool = {};
	descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool.pNext = NULL;
	descriptor_pool.maxSets = 1;
	descriptor_pool.poolSizeCount = 1;
	descriptor_pool.pPoolSizes = type_count;

	VK_CHECK( vkCreateDescriptorPool(data.device, &descriptor_pool, NULL, &data.descriptorPool) );

	VkDescriptorSetAllocateInfo alloc_info[1];
	alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info[0].pNext = NULL;
	alloc_info[0].descriptorPool = data.descriptorPool;
	alloc_info[0].descriptorSetCount = NUM_DESCRIPTOR_SETS;
	alloc_info[0].pSetLayouts = data.descriptorLayouts.data();
	data.descriptorSets.resize(NUM_DESCRIPTOR_SETS);
	VK_CHECK( vkAllocateDescriptorSets(data.device, alloc_info, data.descriptorSets.data()) );


	VkWriteDescriptorSet writes[1];
	writes[0] = {};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].pNext = NULL;
	writes[0].dstSet = data.descriptorSets[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo = &data.uniformBuffer.descriptorInfo;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;

	vkUpdateDescriptorSets(data.device, 1, writes, 0, NULL);

}

void CreateRenderPass(VulkanData& data)
{
	VkAttachmentDescription attachments[2];
	attachments[0].format = data.swapchainFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachments[0].flags = 0;

	attachments[1].format = VK_FORMAT_D16_UNORM;
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
	VK_CHECK( vkCreateRenderPass(data.device, &rp_info, NULL, &data.renderPass) );
}






void CreateShaders(VulkanData& data)
{

	static const char *vertShaderText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout (std140, binding = 0) uniform bufferVals {\n"
		"    mat4 mvp;\n"
		"} myBufferVals;\n"
		"layout (location = 0) in vec4 pos;\n"
		"layout (location = 1) in vec4 inColor;\n"
		"layout (location = 0) out vec4 outColor;\n"
		"void main() {\n"
		"   outColor = inColor;\n"
		"   gl_Position = myBufferVals.mvp * pos;\n"
		"}\n";

	static const char *fragShaderText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout (location = 0) in vec4 color;\n"
		"layout (location = 0) out vec4 outColor;\n"
		"void main() {\n"
		"   outColor = color;\n"
		"}\n";

	
	BeginCompilation();

	VkPipelineShaderStageCreateInfo vertexStage;
	std::vector<unsigned int> vtx_spv;
	vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexStage.pNext = NULL;
	vertexStage.pSpecializationInfo = NULL;
	vertexStage.flags = 0;
	vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexStage.pName = "main";

	auto result = GLSLtoSPV(VK_SHADER_STAGE_VERTEX_BIT, vertShaderText, vtx_spv);
	assert(result);

	VkShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = vtx_spv.size() * sizeof(unsigned int);
	moduleCreateInfo.pCode = vtx_spv.data();
	VK_CHECK( vkCreateShaderModule(data.device, &moduleCreateInfo, NULL, &vertexStage.module) );


	VkPipelineShaderStageCreateInfo fragmentStage;

	std::vector<unsigned int> frag_spv;
	fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentStage.pNext = NULL;
	fragmentStage.pSpecializationInfo = NULL;
	fragmentStage.flags = 0;
	fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentStage.pName = "main";

	result = GLSLtoSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderText, frag_spv);
	assert(result);

	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = frag_spv.size() * sizeof(unsigned int);
	moduleCreateInfo.pCode = frag_spv.data();
	VK_CHECK( vkCreateShaderModule(data.device, &moduleCreateInfo, NULL, &fragmentStage.module) );

	data.vertexStage = vertexStage;
	data.fragmentStage = fragmentStage;

	EndCompilation();
}


static VulkanData s_vulkanData;

void InitVulkan(sf::Window* window)
{
	CreateVulkanInstance(s_vulkanData);

	InitDebugReports(s_vulkanData.instance);

	CreateSurface(window, s_vulkanData);

	CreateVulkanDevice(s_vulkanData);
	CreateVulkanCmdBuffer(s_vulkanData);

	CreateSwapchain(s_vulkanData, window);

	CreateDepthBuffer(s_vulkanData, window->getSize().x, window->getSize().y);

	CreateUniformBuffer(s_vulkanData);

	CreateRenderPass(s_vulkanData);

	CreateShaders(s_vulkanData);
}

