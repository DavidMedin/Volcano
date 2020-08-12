#include "devices.h"

//requested extentions for our GPU
const char* requiredDeviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


//not using device details because this function is called before packing (we don't know which device to use yet)
//capabilities will be written too
// returns 1 if there is an available format and present mode, else returns 0
int DeviceGetSwapChainDetails(VkPhysicalDevice phyDev,VkSurfaceKHR surface,SwapChainSupportDetails* swapDets) {
	//get device capabilites
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyDev, surface, &(swapDets->capabilities));
	//get formats
	swapDets->formats = NULL;
	swapDets->formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(phyDev, surface, &swapDets->formatCount, NULL);
	if (swapDets->formatCount != 0) {
		swapDets->formats = malloc(sizeof(VkSurfaceFormatKHR) * swapDets->formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(phyDev, surface, &swapDets->formatCount, swapDets->formats);
	}

	//get preset modes
	swapDets->presentModes = NULL;
	swapDets->presentCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(phyDev, surface, &swapDets->presentCount, NULL);
	if (swapDets->presentCount != 0) {
		swapDets->presentModes = malloc(sizeof(VkPresentModeKHR) * swapDets->presentCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(phyDev, surface, &swapDets->presentCount, swapDets->presentModes);
	}
	return swapDets->formatCount > 0 && swapDets->presentCount > 0;
}

//not asking for device details because this is a check, before packaging into device details
int HasRequiredQueueFamilies(VkPhysicalDevice phyDev,VkSurfaceKHR surface,QueueFamilyIndex* queueFamilies) {
	//get the queue families the device has
	unsigned int queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(phyDev, &queueCount, NULL);
	VkQueueFamilyProperties* queues = malloc(sizeof(VkQueueFamilyProperties) * queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(phyDev, &queueCount, queues);

	queueFamilies->familyCount = 0;
	queueFamilies->exists = 0;
	for (unsigned int i = 0; i < queueCount; i++) {
		//check which one is for graphics calls
		VkBool32 presentable = 0;
		vkGetPhysicalDeviceSurfaceSupportKHR(phyDev, i, surface, &presentable);
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
			Error("Physical Device doesn't have extension: %s\n", requestedExtentions[j]);
			Error("Physical Device only has these extentions!\n");
			for (unsigned int q = 0; q < extentionCount; q++) {
				Error("	%s\n", extentionProps[q].extensionName);
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
		if(dets->formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && dets->formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return i;
		}
	}
	Error("the ideal colorspace wasn't provided in formats!\n");
	return 0;
}

unsigned int Swap_GetBestPresentMode(SwapChainSupportDetails* dets){
	for(unsigned int i=0;i < dets->presentCount;i++){
		if(dets->presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR){
			return i;
		}
	}
	Error("no triple buffer present mode, falling back to double buffer");
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
//device,imageCount,images,and imageFormat is input
//imageViews is an output arguments
int CreateImageViews(VkDevice device,unsigned int imageCount,VkImage* images,VkSurfaceFormatKHR* imageFormat,VkImageView** imageViews){
	if(imageCount > 0){
		*imageViews = malloc(sizeof(VkImageView*)*imageCount);
		for(unsigned int i =0;i < imageCount;i++){
			//create the creation struct
			VkImageViewCreateInfo imageInfo = {0};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageInfo.image = images[i];
			//we are dealing with 2d things here
			imageInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageInfo.format = imageFormat->format;

			imageInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			//describes image access and use
			imageInfo.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;//useed for color?
			imageInfo.subresourceRange.baseMipLevel = 0;
			imageInfo.subresourceRange.levelCount = 1;
			imageInfo.subresourceRange.baseArrayLayer = 0;
			imageInfo.subresourceRange.layerCount = 1;
			if(vkCreateImageView(device,&imageInfo,NULL,&(*imageViews)[i])!= VK_SUCCESS){
				Error("Couldn't create a imageView!\n");
				return 0;
			}
		}
	}else{
		Error("CreateImageViews was given 0 as an imageCount! that is wrong.\n");
		return 0;
	}
	return 1;
}
int CreateSwapChain(VkDevice device,QueueFamilyIndex* families,VkSurfaceKHR surface, SwapChain* swapChain){
	swapChain->chosenFormat = Swap_GetBestSurfaceFormat(&swapChain->swapDets);
	swapChain->chosenPresent = Swap_GetBestPresentMode(&swapChain->swapDets);
	VkSurfaceFormatKHR* format = &(swapChain->swapDets.formats[swapChain->chosenFormat]);
	VkPresentModeKHR present = swapChain->swapDets.presentModes[swapChain->chosenPresent];
	VkExtent2D extent = Swap_GetBestSwapExtent(&swapChain->swapDets);

	unsigned int imageCount = swapChain->swapDets.capabilities.minImageCount + 1;
	//if maximage is 0, unlimited images; otherwise get the min of the two
	if (swapChain->swapDets.capabilities.maxImageCount > 0 && imageCount > swapChain->swapDets.capabilities.maxImageCount) {
		imageCount = swapChain->swapDets.capabilities.maxImageCount;
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

	//taking the availability of .graphics and .presentation for granted
		if(families->presentation == families->graphics){
		swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//only one family can push to the swapchain
		swapInfo.queueFamilyIndexCount = 0;
		swapInfo.pQueueFamilyIndices = NULL;
	}else{
		swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//more than one can push, need to juggle permissions tho
		swapInfo.queueFamilyIndexCount = 2;
		unsigned int indices[] = {families->graphics,families->presentation};
		swapInfo.pQueueFamilyIndices = indices;
	}

	swapInfo.preTransform = swapChain->swapDets.capabilities.currentTransform; // no applied transform
	swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //no alpha blending with other windows

	swapInfo.presentMode = present;
	swapInfo.clipped = VK_TRUE;//don't care about pixels that are obstructed by other windows

	swapInfo.oldSwapchain = VK_NULL_HANDLE;//for when we need to change the swapchain (like when we need to resize the window)

	if(vkCreateSwapchainKHR(device,&swapInfo,NULL,&swapChain->swapChain) != VK_SUCCESS){
		Error("couldn't create a swap chain! oh no boo hoo.\n");
		return 0;
	}
	//get the VkImages
	swapChain->imageCount = 0;
	vkGetSwapchainImagesKHR(device,swapChain->swapChain,&swapChain->imageCount,NULL);
	swapChain->images = malloc(sizeof(VkImage)*swapChain->imageCount);
	vkGetSwapchainImagesKHR(device,swapChain->swapChain,&swapChain->imageCount,swapChain->images);
	if(swapChain->imageCount == 0){
		Error("the swap chain has zero images!\n");
	}

	if(!CreateImageViews(device,swapChain->imageCount,swapChain->images,&swapChain->swapDets.formats[swapChain->chosenFormat],&swapChain->imageViews)){
		Error("Couldn't create image views!\n");
		return 0;
	}
	swapChain->swapExtent = extent;
	return 1;
}


//inputs an instance and a physical device shell
//returns a physical device thru that shell and queue family info on that device
int GetPhysicalDevice(VkInstance instance,VkSurfaceKHR surface,QueueFamilyIndex* families, SwapChainSupportDetails* dets,VkPhysicalDevice* phyDev) { // graphics card for example
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);//query the num of devices found
	if (deviceCount == 0) {
		Error("No devices found, you poor dood!\n");
		return 0;
	}
	//a pointer to an array of device pointers
	VkPhysicalDevice* deviceList = malloc(sizeof(VkPhysicalDevice) * deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList);
	//pick out the best device
	dets->formats = NULL;
	dets->presentModes = NULL;
	for (unsigned int i = 0; i < deviceCount; i++) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(deviceList[i], &props);

		//check if device is suitable for graphics use
			//has a late enough Vulkan api
			//is a descrete (not integrated) GPU
			//has the requested queue families (graphics, presentation, etc.)
			//the device has available extentions (like for surfaces)
			//the swap chain has an available format and present mode
		if (props.apiVersion >= VK_API_VERSION_1_2 && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && HasRequiredQueueFamilies(deviceList[i],surface, families) && DeviceHasRequiredExtentions(deviceList[i], (char**)requiredDeviceExtensions) && DeviceGetSwapChainDetails(deviceList[i], surface, dets)) {//conditions device must pass
			//found it!
			*phyDev = deviceList[i];
			break;
		}
		else {
			if (dets->formats != NULL) free(dets->formats);
			if (dets->presentModes != NULL) free(dets->presentModes);
		}
	}
	if (phyDev == NULL) {
		Error("There are no devices that match our requirements!\n");
		return 0;
	}
	return 1;
}

//creates the CreateInfo for the queue families
//create the device info
//create the device
//input instance,input surface,output phyDevice,output families, output device
int CreateDevices(VkInstance instance,VkSurfaceKHR surface,DeviceDetails* dets) {
	if (!GetPhysicalDevice(instance,surface,&(dets->families),&(dets->swapChain.swapDets),&(dets->phyDev))) {
		return 0;
	}
	//Creating a graphics logical device-------------------------------------
	//create info about queues to be created
	float priorities = 1.0f;//everything might just be 1

	VkDeviceQueueCreateInfo* queueInfos = malloc(sizeof(VkDeviceQueueCreateInfo) * dets->families.familyCount);
	unsigned int nextIndex = 0;
	if (dets->families.exists & GRAPHICS_BIT) {
		queueInfos[nextIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfos[nextIndex].queueFamilyIndex = dets->families.graphics;
		queueInfos[nextIndex].queueCount = 1;
		queueInfos[nextIndex].pQueuePriorities = &priorities;//this can be an array in the future
		queueInfos[nextIndex].pNext = NULL;
		queueInfos[nextIndex].flags = 0;
		nextIndex++;
	}
	//-------------------------
	if (dets->families.exists & PRESENTATION_BIT && dets->families.graphics != dets->families.presentation) {
		queueInfos[nextIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfos[nextIndex].queueFamilyIndex = dets->families.presentation;
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
	deviceInfo.queueCreateInfoCount = dets->families.familyCount;//when we add more queues we need to inc this
	deviceInfo.pQueueCreateInfos = queueInfos;
	deviceInfo.flags = 0;
	deviceInfo.pNext = NULL;
	if (vkCreateDevice(dets->phyDev, &deviceInfo, NULL, &(dets->device)) != VK_SUCCESS) {
		Error("Couldn't create a logical device. oh no\n");
	}

	//create the swapchain for the device
	if(!CreateSwapChain(dets->device,&(dets->families),surface,&(dets->swapChain))){
		return 0;
	}	

	return 1;
}
