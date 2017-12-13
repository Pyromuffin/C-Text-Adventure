
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

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>




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

struct VulkanData
{
	int framebufferWidth;
	int framebufferHeight;

	VkInstance instance;
	VkPhysicalDevice gpu;
	VkDevice device;
	VkPhysicalDeviceMemoryProperties memoryProps;

	uint32_t queueFamilyIndex;
	VkQueue queue;

	VkCommandPool cmdBufferPool;
	VkCommandBuffer cmdBuffer;

	VkSurfaceKHR presentSurface;
	VkFormat swapchainFormat;
	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	uint32_t currentSwapchainIndex;
	VkSemaphore imageAcquireSemaphore;
	VkFence frameFence;

	GpuImage depthBuffer;
	GpuBuffer uniformBuffer;

	std::vector<VkDescriptorSetLayout> descriptorLayouts;
	std::vector<VkDescriptorSet> descriptorSets;
	VkPipelineLayout pipelineLayout;
	VkDescriptorPool descriptorPool;
	VkSampler linearClampSampler;

	VkRenderPass renderPass;

	VkPipelineShaderStageCreateInfo vertexStage;
	VkPipelineShaderStageCreateInfo fragmentStage;

	VkFramebuffer framebuffers[2];
	VkViewport viewport;
	VkRect2D scissor;

	GpuBuffer vertexBuffer;
	VkVertexInputBindingDescription vertexBinding;
	VkVertexInputAttributeDescription vertexAttributes[2];

	VkPipeline pipeline;
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

void CreateVulkanInstance(VulkanData &data)
{
	std::vector<const char*> instanceExtensions;
	init_instance_extension_names(instanceExtensions);
	instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

	std::vector<const char*> instanceLayers;
	instanceLayers.push_back(VALIDATION_LAYER_NAME);

	VkInstanceCreateInfo info;
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.enabledLayerCount = instanceLayers.size();
	info.ppEnabledLayerNames = instanceLayers.data();
	info.enabledExtensionCount = instanceExtensions.size();
	info.ppEnabledExtensionNames = instanceExtensions.data();

	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "C Text Adventure";
	appInfo.pEngineName = "SUPER ENGINE NAME";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	info.pApplicationInfo = &appInfo;

	if (!CheckValidationLayerSupport(instanceLayers)) 
	{
		printf("validation busted\n");
		assert(false);
	}

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
	for(int i = 0; i < queueFamilyCount; i++)
	{
		if (queueProps[i].queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT != 0)
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

	float queue_priorities[1] = { 1.0f };

	VkDeviceQueueCreateInfo queueInfo;
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pNext = nullptr;
	queueInfo.flags = 0;
	queueInfo.queueCount = 1;
	queueInfo.queueFamilyIndex = familyIndex;
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

	vkGetDeviceQueue(data.device, familyIndex, 0, &data.queue);
	vkGetPhysicalDeviceMemoryProperties(data.gpu, &data.memoryProps);
}






void CreateVulkanCmdBuffer(VulkanData& data)
{

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.pNext = NULL;
	cmdPoolInfo.queueFamilyIndex = data.queueFamilyIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;


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

static VkResult __s_debugResult;
#define VK_CHECK( func ) \
__s_debugResult = func; \
assert(__s_debugResult == VK_SUCCESS )

void CreateDepthBuffer(VulkanData& data)
{
	VkImageCreateInfo image_info;

	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = NULL;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = VK_FORMAT_D16_UNORM;
	image_info.extent.width = data.framebufferWidth;
	image_info.extent.height = data.framebufferHeight;
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

	
	VK_CHECK( vkAllocateMemory(data.device, &mem_alloc, nullptr, &data.depthBuffer.memory) );
	VK_CHECK( vkBindImageMemory(data.device, data.depthBuffer.image, data.depthBuffer.memory, 0) );
	VK_CHECK( vkCreateImageView(data.device, &view_info, nullptr, &data.depthBuffer.view) );
	

}

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

void CreateDescriptors(VulkanData& data)
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
		"layout (std140, binding = 0) uniform buf {\n"
		"        mat4 mvp;\n"
		"} ubuf;\n"
		"layout (location = 0) in vec4 pos;\n"
		"layout (location = 1) in vec2 inTexCoords;\n"
		"layout (location = 0) out vec2 texcoord;\n"
		"void main() {\n"
		"   texcoord = inTexCoords;\n"
		"   gl_Position = ubuf.mvp * pos;\n"
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
	

	VkPipelineShaderStageCreateInfo vertexStage;
	vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexStage.pNext = NULL;
	vertexStage.pSpecializationInfo = NULL;
	vertexStage.flags = 0;
	vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexStage.pName = "main";

	auto vertexShaderBin = CompileShader(vertShaderText, shaderc_shader_kind::shaderc_vertex_shader, "My Vertex Shader");
	assert(vertexShaderBin.size() > 0);

	VkShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = vertexShaderBin.size() * sizeof(uint32_t);
	moduleCreateInfo.pCode = vertexShaderBin.data();
	VK_CHECK( vkCreateShaderModule(data.device, &moduleCreateInfo, NULL, &vertexStage.module) );


	VkPipelineShaderStageCreateInfo fragmentStage;
	fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentStage.pNext = NULL;
	fragmentStage.pSpecializationInfo = NULL;
	fragmentStage.flags = 0;
	fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentStage.pName = "main";

	auto fragmentShaderBin = CompileShader(fragShaderText, shaderc_shader_kind::shaderc_fragment_shader, "My Fragment Shader");
	assert(fragmentShaderBin.size() > 0);

	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = fragmentShaderBin.size() * sizeof(uint32_t);
	moduleCreateInfo.pCode = fragmentShaderBin.data();
	VK_CHECK( vkCreateShaderModule(data.device, &moduleCreateInfo, NULL, &fragmentStage.module) );

	data.vertexStage = vertexStage;
	data.fragmentStage = fragmentStage;
}

void CreateFrameBuffer(VulkanData& data)
{
	VkImageView attachments[2];
	attachments[1] = data.depthBuffer.view;

	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = NULL;
	fb_info.renderPass = data.renderPass;
	fb_info.attachmentCount = 2;
	fb_info.pAttachments = attachments;
	fb_info.width = data.framebufferWidth;
	fb_info.height = data.framebufferHeight;
	fb_info.layers = 1;

	attachments[0] = data.swapchainImageViews[0];
	VK_CHECK( vkCreateFramebuffer(data.device, &fb_info, NULL, &data.framebuffers[0]) );

	attachments[0] = data.swapchainImageViews[1];
	VK_CHECK( vkCreateFramebuffer(data.device, &fb_info, NULL, &data.framebuffers[1]) );
}

#include "cube_data.h"

template<typename T>
VkMemoryRequirements GetMemoryRequirements(VkDevice device, T obj)
{
	VkMemoryRequirements mem_reqs;

	if constexpr(std::is_same<T, GpuBuffer>().value)
	{
		vkGetBufferMemoryRequirements(device, obj.buffer, &mem_reqs);
		return mem_reqs;
	}
	else if constexpr(std::is_same<T, GpuImage>().value)
	{
		vkGetImageMemoryRequirements(device, obj.image, &mem_reqs);
		return mem_reqs;
	}
	else constexpr
	{
		assert(false && "Invalid GPU allocation type" );
	}
}

static VkPhysicalDeviceMemoryProperties s_memoryProps;

template<typename T>
void AllocateGpuMemory(VkDevice device, T& obj, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProps = s_memoryProps)
{
	VkMemoryRequirements reqs = GetMemoryRequirements(device, obj);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.memoryTypeIndex = 0;
	alloc_info.allocationSize = reqs.size;

	auto pass = GetMemoryTypeIndexFromProps(memoryProps, reqs.memoryTypeBits, memoryFlags, &alloc_info.memoryTypeIndex);
	assert(pass && "No mappable, coherent memory");

	VK_CHECK( vkAllocateMemory(device, &alloc_info, NULL, &obj.memory) );
	
	obj.memorySize = reqs.size;
}

void CreateVertexBuffer(VulkanData& data)
{
	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buf_info.size = sizeof(g_vb_texture_Data);
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = NULL;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;
	
	VK_CHECK( vkCreateBuffer(data.device, &buf_info, NULL, &data.vertexBuffer.buffer) );

	
	AllocateGpuMemory(data.device, data.vertexBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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

void CreatePipeline(VulkanData& data)
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
	vi.pVertexBindingDescriptions = &data.vertexBinding;
	vi.vertexAttributeDescriptionCount = 2;
	vi.pVertexAttributeDescriptions = data.vertexAttributes;

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

	VkGraphicsPipelineCreateInfo pipeline;
	pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline.pNext = NULL;
	pipeline.layout = data.pipelineLayout;
	pipeline.basePipelineHandle = VK_NULL_HANDLE;
	pipeline.basePipelineIndex = 0;
	pipeline.flags = 0;
	pipeline.pVertexInputState = &vi;
	pipeline.pInputAssemblyState = &ia;
	pipeline.pRasterizationState = &rs;
	pipeline.pColorBlendState = &cb;
	pipeline.pTessellationState = NULL;
	pipeline.pMultisampleState = &ms;
	pipeline.pDynamicState = &dynamicState;
	pipeline.pViewportState = &vp;
	pipeline.pDepthStencilState = &ds;

	VkPipelineShaderStageCreateInfo stages[] = { data.vertexStage, data.fragmentStage };
	pipeline.pStages = stages;
	pipeline.stageCount = 2;
	pipeline.renderPass = data.renderPass;
	pipeline.subpass = 0;

	VK_CHECK( vkCreateGraphicsPipelines(data.device, NULL, 1, &pipeline, NULL, &data.pipeline) );
}

VkCommandBuffer BeginRenderPass(VulkanData& data)
{
	// Get the index of the next available swapchain image:
	VK_CHECK(vkAcquireNextImageKHR(data.device, data.swapchain, UINT64_MAX, data.imageAcquireSemaphore, VK_NULL_HANDLE, &data.currentSwapchainIndex));

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

	vkResetFences(data.device, 1, &data.frameFence);

	return data.cmdBuffer;
}


void EndRenderPass(VkCommandBuffer cmdBuffer)
{
	vkCmdEndRenderPass(cmdBuffer);
	VK_CHECK(vkEndCommandBuffer(cmdBuffer));
}

void SubmitCommandBuffers(VulkanData& data, VkCommandBuffer* cmdBuffers, uint32_t commandBufferCount, VkFence fence)
{
	VkPipelineStageFlags pipe_stage_flags =
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo submit_info[1] = {};
	submit_info[0].pNext = NULL;
	submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info[0].waitSemaphoreCount = 1;
	submit_info[0].pWaitSemaphores = &data.imageAcquireSemaphore;
	submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
	submit_info[0].commandBufferCount = commandBufferCount;
	submit_info[0].pCommandBuffers = cmdBuffers;
	submit_info[0].signalSemaphoreCount = 0;
	submit_info[0].pSignalSemaphores = NULL;
	VK_CHECK(vkQueueSubmit(data.queue, 1, submit_info, fence));
}

bool WaitForFence(VulkanData& data, VkFence fence, uint64_t timeoutNanos)
{
	auto res = vkWaitForFences(data.device, 1, &fence, VK_TRUE, timeoutNanos);
	return res == VK_TIMEOUT;
}

void SetViewportAndScissor(VulkanData& data, VkCommandBuffer cmdBuffer, sf::Vector2u size, sf::Vector2u offset)
{
	data.viewport.width = static_cast<float>(size.x);
	data.viewport.height = static_cast<float>(size.y);
	data.viewport.minDepth = 0.0f;
	data.viewport.maxDepth = 1.0f;
	data.viewport.x = static_cast<float>(offset.x);
	data.viewport.y = static_cast<float>(offset.y);
	vkCmdSetViewport(cmdBuffer, 0, 1, &data.viewport);

	data.scissor.extent.width = size.x;
	data.scissor.extent.height = size.y;
	data.scissor.offset.x = offset.x;
	data.scissor.offset.y = offset.y;
	vkCmdSetScissor(cmdBuffer, 0, 1, &data.scissor);
}

void Present(VulkanData& data)
{
	VkPresentInfoKHR present;
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pNext = NULL;
	present.swapchainCount = 1;
	present.pSwapchains = &data.swapchain;
	present.pImageIndices = &data.currentSwapchainIndex;
	present.pWaitSemaphores = NULL;
	present.waitSemaphoreCount = 0;
	present.pResults = NULL;
	VK_CHECK(vkQueuePresentKHR(data.queue, &present));
}


void Cube(VulkanData& data)
{
	VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
	imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	imageAcquiredSemaphoreCreateInfo.pNext = NULL;
	imageAcquiredSemaphoreCreateInfo.flags = 0;

	VK_CHECK( vkCreateSemaphore(data.device, &imageAcquiredSemaphoreCreateInfo, NULL, &data.imageAcquireSemaphore) );

	// Get the index of the next available swapchain image:
	VK_CHECK( vkAcquireNextImageKHR(data.device, data.swapchain, UINT64_MAX, data.imageAcquireSemaphore, VK_NULL_HANDLE, &data.currentSwapchainIndex) );

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

	VkFenceCreateInfo fenceInfo;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = 0;
	vkCreateFence(data.device, &fenceInfo, NULL, &data.frameFence);

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

void UploadTexture(unsigned char* texture, int x, int y);

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


GpuBuffer CreateBuffer(VulkanData& data, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryProperties)
{
	GpuBuffer buffer;

	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.usage = usageFlags;
	buf_info.size = size;
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = NULL;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;
	VK_CHECK( vkCreateBuffer(data.device, &buf_info, NULL, &buffer.buffer) );

	AllocateGpuMemory(data.device, buffer, memoryProperties);

	VK_CHECK(vkBindBufferMemory(data.device, buffer.buffer, buffer.memory, 0));

	return buffer;
}


VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format)
{
	VkImageViewCreateInfo color_image_view = {};
	color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	color_image_view.pNext = NULL;
	color_image_view.flags = 0;
	color_image_view.image = image;
	color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	color_image_view.format = format;
	color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
	color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
	color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
	color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
	color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	color_image_view.subresourceRange.baseMipLevel = 0;
	color_image_view.subresourceRange.levelCount = 1;
	color_image_view.subresourceRange.baseArrayLayer = 0;
	color_image_view.subresourceRange.layerCount = 1;

	VkImageView view;

	VK_CHECK(vkCreateImageView(device, &color_image_view, NULL, &view));
	return view;
}


GpuImage CreateImage(VulkanData& data, uint32_t x, uint32_t y, VkFormat format, VkImageTiling tiling, VkSampler sampler, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryProperties)
{
	GpuImage image;
	VkImageCreateInfo image_info;

	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = NULL;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = format;
	image_info.extent.width = x;
	image_info.extent.height = y;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = usageFlags;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = NULL;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.flags = 0;
	image_info.tiling = tiling;

	VK_CHECK( vkCreateImage(data.device, &image_info, nullptr, &image.image) );

	AllocateGpuMemory(data.device, image, memoryProperties);
	VK_CHECK( vkBindImageMemory(data.device, image.image, image.memory, 0) );

	image.view = CreateImageView(data.device, image.image, format);

	VkDescriptorImageInfo descriptorImageInfo = {};
	descriptorImageInfo.sampler = sampler;
	descriptorImageInfo.imageView = image.view;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	image.descriptorInfo = descriptorImageInfo;

	return image;
}

void CopyBufferToImage(VkCommandBuffer cmdBuffer, VkBuffer srcBuffer, VkImage dstImage, uint32_t x, uint32_t y)
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


void TransitionImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout )
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	barrier.srcAccessMask = 0; // handled below
	barrier.dstAccessMask = 0; 

	VkPipelineStageFlags sourceStage, destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

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
		1, &barrier
	);
}


void UploadTexture(unsigned char* texture, int x, int y)
{
	// implicit usage of data for now
	auto& data = s_vulkanData;
	size_t imageSize = x * y * sizeof(char);

	auto stagingBuffer = CreateBuffer(data, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* cpuMemory;
	vkMapMemory(data.device, stagingBuffer.memory, 0, stagingBuffer.memorySize, 0, &cpuMemory);
	memcpy(cpuMemory, texture, imageSize);
	vkUnmapMemory(data.device, stagingBuffer.memory);

	auto gpuTexture = CreateImage(data, x, y, VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL, data.linearClampSampler, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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

	vkUpdateDescriptorSets(data.device, 2, writes, 0, NULL);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	vkBeginCommandBuffer(data.cmdBuffer, &beginInfo);
	TransitionImage(data.cmdBuffer, gpuTexture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage(data.cmdBuffer, stagingBuffer.buffer, gpuTexture.image, x, y);
	TransitionImage(data.cmdBuffer, gpuTexture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	vkEndCommandBuffer(data.cmdBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &data.cmdBuffer;

	vkQueueSubmit(data.queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(data.queue);
}