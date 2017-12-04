
#include <cstdio>

#ifdef _WIN32
	#include <Windows.h>
	#ifndef VK_USE_PLATFORM_WIN32_KHR
		#define VK_USE_PLATFORM_WIN32_KHR
	#endif
#endif

#include <SFML/Window.hpp>

#include <vulkan/vulkan.hpp>


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


VkSurfaceKHR CreateSurface(sf::Window* window, VkInstance instance)
{
VkResult res;
VkSurfaceKHR surface;


#ifdef _WIN32
	auto handle = window->getSystemHandle();
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(handle, GWLP_HINSTANCE);

	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.hinstance = hInstance;
	createInfo.hwnd = handle;
	res = vkCreateWin32SurfaceKHR(instance, &createInfo, NULL, &surface);
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
	return surface;
}



VkInstance CreateVulkanInstance()
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

	return instance;
}


VkDevice CreateVulkanDevice(VkInstance instance, VkPhysicalDevice* outGPU, uint32_t* outQueueFamilyIndex, VkSurfaceKHR presentSurface)
{
	std::vector<const char*> deviceExtensions;
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	uint32_t gpuCount;
	std::vector<VkPhysicalDevice> gpus;

	// Get the number of devices (GPUs) available.
	auto res = vkEnumeratePhysicalDevices(instance, &gpuCount, NULL);
	// Allocate space and get the list of devices.
	gpus.resize(gpuCount);
	res = vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data());

	if (res)
	{
		printf("Vulkan enumerate error\n");
	}

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

	// now we assume that this queue can present or else.
	VkBool32 canPresent;
	vkGetPhysicalDeviceSurfaceSupportKHR(gpus[0], familyIndex, presentSurface, &canPresent);
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

	*outGPU = gpus[0];
	*outQueueFamilyIndex = familyIndex;
	return device;
}






VkCommandBuffer CreateVulkanCmdBuffer(VkDevice device, uint32_t familyIndex)
{

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.pNext = NULL;
	cmdPoolInfo.queueFamilyIndex = familyIndex;
	cmdPoolInfo.flags = 0;


	VkCommandPool cmdPool;
	auto res = vkCreateCommandPool(device, &cmdPoolInfo, NULL, &cmdPool);

	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = NULL;
	cmd.commandPool = cmdPool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	VkCommandBuffer cmdBuffer;

	res = vkAllocateCommandBuffers(device, &cmd, &cmdBuffer);
	assert(res == VK_SUCCESS);

	return cmdBuffer;
}


VkSwapchainKHR CreateSwapchain( VkSurfaceKHR presentSurface, VkPhysicalDevice gpu, VkDevice device, sf::Window* window, std::vector<VkImage>& swapchainImages)
{
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, presentSurface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats;
	formats.resize(formatCount);

	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, presentSurface, &formatCount, formats.data());

	auto surfaceFormat = formats[0].format;

	VkSurfaceCapabilitiesKHR surfaceCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, presentSurface, &surfaceCaps);

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, presentSurface, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, presentSurface, &presentModeCount, presentModes.data());

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
	swapchain_ci.surface = presentSurface;
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
	auto res = vkCreateSwapchainKHR(device, &swapchain_ci, NULL, &swapchain);
	assert(res == VK_SUCCESS);

	uint32_t swapchainImageCount;
	res = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, NULL);
	assert(res == VK_SUCCESS);

	swapchainImages.resize(swapchainImageCount);

	res = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data());
	assert(res == VK_SUCCESS);


	return swapchain;
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
		SuperDebug,                                        // pfnCallback
		NULL                                                        // pUserData
	};

	PFN_vkCreateDebugReportCallbackEXT createDebugReport = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	auto res = createDebugReport(instance, &callback1, nullptr, &cb1);
}


void InitVulkan(sf::Window* window)
{
	auto instance = CreateVulkanInstance();

	InitDebugReports(instance);

	auto presentSurface = CreateSurface(window, instance);

	uint32_t familyIndex;
	VkPhysicalDevice gpu;
	auto device = CreateVulkanDevice(instance, &gpu, &familyIndex, presentSurface);
	auto cmdBuffer = CreateVulkanCmdBuffer(device, familyIndex);

	std::vector<VkImage> swapchainImages;
	CreateSwapchain(presentSurface, gpu, device, window, swapchainImages);
}

