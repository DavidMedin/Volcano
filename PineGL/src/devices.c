#include "devices.h"

//requested extentions for our GPU
const char* requiredDeviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


//capabilities will be written too
// returns 1 if there is an available format and present mode, else returns 0
int DeviceGetSwapChainDetails(VkPhysicalDevice device,VkSurfaceKHR surface, SwapChainSupportDetails* details) {
	//get device capabilites
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details->capabilities);
	//get formats
	details->formats = NULL;
	details->formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details->formatCount, NULL);
	if (details->formatCount != 0) {
		details->formats = malloc(sizeof(VkSurfaceFormatKHR) * details->formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details->formatCount, details->formats);
	}

	//get preset modes
	details->presentModes = NULL;
	details->presentCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details->presentCount, NULL);
	if (details->presentCount != 0) {
		details->presentModes = malloc(sizeof(VkPresentModeKHR) * details->presentCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details->presentCount, details->presentModes);
	}
	return details->formatCount > 0 && details->presentCount > 0;
}


int HasRequiredQueueFamilies(VkPhysicalDevice device,VkSurfaceKHR surface, QueueFamilyIndex* queueFamilies) {
	//get the queue families the device has
	unsigned int queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, NULL);
	VkQueueFamilyProperties* queues = malloc(sizeof(VkQueueFamilyProperties) * queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queues);

	queueFamilies->familyCount = 0;
	queueFamilies->exists = 0;
	for (unsigned int i = 0; i < queueCount; i++) {
		//check which one is for graphics calls
		VkBool32 presentable = 0;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentable);
		if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			queueFamilies->graphics = i;
			queueFamilies->exists |= GRAPHICS_BIT;
			queueFamilies->familyCount++;
		}
		if (presentable && !(queueFamilies->exists & PRESENTATION_BIT)) {
			//this queue also supports the presentation queue
			queueFamilies->presentation = i;
			queueFamilies->exists |= PRESENTATION_BIT;
			queueFamilies->graphics == queueFamilies->presentation ? 1 : queueFamilies->familyCount++;
		}
	}
	free(queues);
	if (queueFamilies->exists == 0) {
		//none of the families in this device were for graphics! oh no!
		return 0;
	}
	else return 1;

}

int DeviceHasRequiredExtentions(VkPhysicalDevice device,char** requestedExtentions) {
	unsigned int extentionCount;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extentionCount, NULL);
	VkExtensionProperties* extentionProps = malloc(sizeof(VkExtensionProperties) * extentionCount);
	vkEnumerateDeviceExtensionProperties(device, NULL, &extentionCount, extentionProps);

	for (unsigned int j = 0; j < sizeof(requestedExtentions) / sizeof(void*); j++) {//go though all required extenstions
		int found = 0;
		for (unsigned int i = 0; i < extentionCount; i++) {//go through all available extenstions
			if (strcmp(extentionProps[i].extensionName, requestedExtentions[j]) == 0) {
				found = 1;
			}
		}
		if (!found) {
			printf("Physical Device doesn't have extension: %s\n", requestedExtentions[j]);
			printf("Physical Device only has these extentions!\n");
			for (unsigned int q = 0; q < extentionCount; q++) {
				printf("	%s\n", extentionProps[q].extensionName);
			}
			free(extentionProps);//failed to find an extension
			return 0;
		}
	}
	free(extentionProps);
	return 1;
}

//returns the index into formats that should be used
unsigned int Swap_GetBestSurfaceFormat(SwapChainSupportDetails* dets){
	for(unsigned int i=0;i < dets->formatCount;i++){
		if(dets->formats[i].format = VK_FORMAT_B8G8R8A8_SRGB && dets->formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return i;
		}
	}
	printf("the ideal colorspace wasn't provided in formats!\n");
	return 0;
}

unsigned int Swap_GetBestPresentMode(SwapChainSupportDetails* dets){
	for(unsigned int i=0;i < dets->presentCount;i++){
		if(dets->presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR){
			return i;
		}
	}
	printf("no triple buffer present mode, falling back to double buffer");
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swap_GetBestSwapExtent(SwapChainSupportDetails* dets){
	if(dets->capabilities.currentExtent.width != UINT32_MAX){
		return dets->capabilities.currentExtent;
	}else{
    	VkExtent2D actualExtent = {WIDTH,HEIGHT};
    	actualExtent.width = min(max(dets->capabilities.minImageExtent.width,actualExtent.width), dets->capabilities.maxImageExtent.width);
    	actualExtent.height = min(max(dets->capabilities.minImageExtent.height,actualExtent.height),dets->capabilities.maxImageExtent.height);
    	return actualExtent;
	}
}

int CreateSwapChain(SwapChainSupportDetails* dets,VkSurfaceKHR surface){
	VkSurfaceFormatKHR* format = &(dets->formats[Swap_GetBestSurfaceFormat(dets)]);
	VkPresentModeKHR* present = &(dets->presentModes[Swap_GetBestPresentMode(dets)]);
	VkExtent2D extent = Swap_GetBestSwapExtent(dets);

	unsigned int imageCount = dets->capabilities.minImageCount + 1;
	//if maximage is 0, unlimited images; otherwise get the min of the two
	if (dets->capabilities.maxImageCount > 0 && imageCount > dets->capabilities.maxImageCount) {
		imageCount = dets->capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR swapInfo = { 0 };
	swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapInfo.surface = surface;
	swapInfo.minImageCount = imageCount;
	swapInfo.imageFormat = format->format;
	swapInfo.imageColorSpace = format->colorSpace;
	swapInfo.imageExtent = extent;
	swapInfo.imageArrayLayers = 1;
	swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


}

//inputs an instance and a physical device shell
//returns a physical device thru that shell and queue family info on that device
int GetPhysicalDevice(VkInstance instance,VkSurfaceKHR surface, PhysicalDeviceDetails* dets) { // graphics card for example
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);//query the num of devices found
	if (deviceCount == 0) {
		printf("No devices found, you poor dood!\n");
		return 0;
	}
	//a pointer to an array of device pointers
	VkPhysicalDevice* deviceList = malloc(sizeof(VkPhysicalDevice) * deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList);
	//pick out the best device
	dets->swapDetails.formats = NULL;
	dets->swapDetails.presentModes = NULL;
	for (unsigned int i = 0; i < deviceCount; i++) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(deviceList[i], &props);

		//check if device is suitable for graphics use
			//has a late enough Vulkan api
			//is a descrete (not integrated) GPU
			//has the requested queue families (graphics, presentation, etc.)
			//the device has available extentions (like for surfaces)
			//the swap chain has an available format and present mode
		if (props.apiVersion >= VK_API_VERSION_1_2 && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && HasRequiredQueueFamilies(deviceList[i],surface, &dets->families) && DeviceHasRequiredExtentions(deviceList[i], requiredDeviceExtensions) && DeviceGetSwapChainDetails(deviceList[i], surface, &dets->swapDetails)) {//conditions device must pass
			//found it!
			dets->phyDev = deviceList[i];
			break;
		}
		else {
			if (dets->swapDetails.formats != NULL) free(dets->swapDetails.formats);
			if (dets->swapDetails.presentModes != NULL) free(dets->swapDetails.presentModes);
		}
	}
	if (dets->phyDev == NULL) {
		printf("There are no devices that match our requirements!\n");
		return 0;
	}
	return 1;
}

//creates the CreateInfo for the queue families
//create the device info
//create the device
//input instance,input surface,output phyDevice,output queueFamilies, output device
int CreateDevices(VkInstance instance,VkSurfaceKHR surface,VkPhysicalDevice* phyDevice,QueueFamilyIndex* queueFamilies,VkDevice* device) {
	if (!GetPhysicalDevice(instance,surface, phyDevice, queueFamilies)) {
		printf("Couldn't create a device!\n");
		return 0;
	}
	//Creating a graphics logical device-------------------------------------
	//create info about queues to be created
	float priorities = 1.0f;//everything might just be 1

	VkDeviceQueueCreateInfo* queueInfos = malloc(sizeof(VkDeviceQueueCreateInfo) * queueFamilies->familyCount);
	unsigned int nextIndex = 0;
	if (queueFamilies->exists & GRAPHICS_BIT) {
		queueInfos[nextIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfos[nextIndex].queueFamilyIndex = queueFamilies->graphics;
		queueInfos[nextIndex].queueCount = 1;
		queueInfos[nextIndex].pQueuePriorities = &priorities;//this can be an array in the future
		queueInfos[nextIndex].pNext = NULL;
		queueInfos[nextIndex].flags = 0;
		nextIndex++;
	}
	//-------------------------
	if (queueFamilies->exists & PRESENTATION_BIT && queueFamilies->graphics != queueFamilies->presentation) {
		queueInfos[nextIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfos[nextIndex].queueFamilyIndex = queueFamilies->presentation;
		queueInfos[nextIndex].queueCount = 1;
		queueInfos[nextIndex].pQueuePriorities = &priorities;//this can be an array in the future
		queueInfos[nextIndex].pNext = NULL;
		queueInfos[nextIndex].flags = 0;
		nextIndex++;
	}
	//create info about the features to be enabled in the device (like geometry shaders)
	VkPhysicalDeviceFeatures deviceFeatures = { 0 };//we'll leave this empty until we need a specific feature

	//create device info
	VkDeviceCreateInfo deviceInfo;
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	if (enableValidationLayers) {//give it the validation layers. will be ignored in recent Vulkan versions tho
		unsigned int layerCount = 0;
		const char** layers = GetInstanceValidationLayers(&layerCount);
		deviceInfo.ppEnabledLayerNames = layers;
		deviceInfo.enabledLayerCount = layerCount;
	}
	else deviceInfo.enabledLayerCount = 0;
	deviceInfo.enabledExtensionCount = 1;
	deviceInfo.ppEnabledExtensionNames = requiredDeviceExtensions;
	deviceInfo.pEnabledFeatures = NULL;
	deviceInfo.queueCreateInfoCount = queueFamilies->familyCount;//when we add more queues we need to inc this
	deviceInfo.pQueueCreateInfos = queueInfos;
	deviceInfo.flags = 0;
	deviceInfo.pNext = NULL;
	if (vkCreateDevice(*phyDevice, &deviceInfo, NULL, device) != VK_SUCCESS) {
		printf("Couldn't create a logical device. oh no\n");
	}
	return 1;
}
