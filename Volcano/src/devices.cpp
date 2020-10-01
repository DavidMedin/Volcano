#include "devices.h"

//requested extentions for our GPU
const char* requiredDeviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


//not using device details because this function is called before packing (we don't know which device to use yet)
//capabilities will be written too
// returns 1 if there is an available format and present mode, else returns 0
int DeviceGetSwapChainDetails(VkPhysicalDevice phyDev,VkSurfaceKHR surface,SwapChainSupportDetails* swapDets) {
	SwapChainSupportDetails _swapDets = {0};
	
	//get device capabilites
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyDev, surface, &(_swapDets.capabilities));
	//get formats
	_swapDets.formats = nullptr;
	_swapDets.formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(phyDev, surface, &_swapDets.formatCount, NULL);
	if (_swapDets.formatCount != 0) {
		_swapDets.formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * _swapDets.formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(phyDev, surface, &_swapDets.formatCount, _swapDets.formats);
	}

	//get preset modes
	_swapDets.presentModes = NULL;
	_swapDets.presentCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(phyDev, surface, &_swapDets.presentCount, NULL);
	if (_swapDets.presentCount != 0) {
		_swapDets.presentModes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * _swapDets.presentCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(phyDev, surface, &_swapDets.presentCount, _swapDets.presentModes);
	}
	int retCondish = _swapDets.formatCount > 0 && _swapDets.presentCount > 0;
	if(swapDets != NULL){
		*swapDets = _swapDets;
		return retCondish;
	}else
		// free(swapDets);
		return retCondish;
}

//not asking for device details because this is a check, before packaging into device details
int HasRequiredQueueFamilies(VkPhysicalDevice phyDev,VkSurfaceKHR surface,QueueFamilyIndex* queueFams) {
	//get the queue families the device has
	QueueFamilyIndex queueFamilies = {0};
	unsigned int queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(phyDev, &queueCount, NULL);
	VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(phyDev, &queueCount, queues);

	queueFamilies.familyCount = 0;
	queueFamilies.exists = 0;
	for (unsigned int i = 0; i < queueCount; i++) {
		//check which one is for graphics calls
		VkBool32 presentable = 0;
		vkGetPhysicalDeviceSurfaceSupportKHR(phyDev, i, surface, &presentable);
		if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			queueFamilies.graphics = i;
			queueFamilies.exists |= GRAPHICS_BIT;
			queueFamilies.familyCount++;
		}
		if (presentable && !(queueFamilies.exists & PRESENTATION_BIT)) {
			//this queue also supports the presentation queue
			queueFamilies.presentation = i;
			queueFamilies.exists |= PRESENTATION_BIT;
			queueFamilies.graphics == queueFamilies.presentation ? 1 : queueFamilies.familyCount++;
		}
	}
	free(queues);
	if (queueFamilies.exists == 0) {
		//none of the families in this device were for graphics! oh no!
		// free(queueFamilies);
		return 0;
	}
	else if(queueFams != NULL) {
			*queueFams = queueFamilies;
			return 1;
		}else{ 
		  	// free(queueFamilies);
	  		return 1;
		}
}

int DeviceHasRequiredExtentions(VkPhysicalDevice device,char** requestedExtentions) {
	unsigned int extentionCount;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extentionCount, NULL);
	VkExtensionProperties* extentionProps = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extentionCount);
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

int IsDeviceCompatible(VkPhysicalDevice phyDev,VkSurfaceKHR surface,VkPhysicalDeviceProperties props,QueueFamilyIndex* fams, SwapChainSupportDetails* swapDets){
	int score = 0;
	if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 3;
	if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score += 2;

	if(props.apiVersion >= VK_API_VERSION_1_2) score += 3;
	else if(props.apiVersion >= VK_API_VERSION_1_1) score += 2;
	else if(props.apiVersion >= VK_API_VERSION_1_0) score += 1;

	//check if device is suitable for graphics use
		//has a late enough Vulkan api
		//is a descrete (not integrated) GPU
		//has the requested queue families (graphics, presentation, etc.)
		//the device has available extentions (like for surfaces)
		//the swap chain has an available format and present mode
	if (HasRequiredQueueFamilies(phyDev,surface, fams) &&
		DeviceHasRequiredExtentions(phyDev, (char**)requiredDeviceExtensions) &&
		DeviceGetSwapChainDetails(phyDev, surface, swapDets))
		return score;
	else
		return 0;	
}


//inputs an instance and a physical device shell
//returns a physical device thru that shell and queue family info on that device
int GetPhysicalDevice(VkInstance instance,VkSurfaceKHR surface,Device* devDets,SwapChainSupportDetails* swapDets) { // graphics card for example
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);//query the num of devices found
	if (deviceCount == 0) {
		Error("No devices found, you poor dood!\n");
		return 0;
	}
	//a pointer to an array of device pointers
	VkPhysicalDevice* deviceList = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList);
	//pick out the best device

	unsigned int bestIndex;
	int bestScore = 0;

	swapDets->formats = NULL;
	swapDets->presentModes = NULL;
	VkPhysicalDeviceProperties props;
	for (unsigned int i = 0; i < deviceCount; i++) {
		vkGetPhysicalDeviceProperties(deviceList[i], &props);

		int tmpScore = IsDeviceCompatible(deviceList[i],surface,props,NULL,NULL);
		if(tmpScore > bestScore){//00000000000000000errorpoint
			bestIndex = i;
			bestScore = tmpScore;
		}

		// if(IsDeviceCompatible(deviceList[i],surface,props,devDets)){
		// 	//found it!
		// 	devDets->phyDev = deviceList[i];
		// 	devDets->phyProps = props;
		// 	break;
		// }
		// else {
		// 	if (swapDets->formats != NULL) free(swapDets->formats);
		// 	if (swapDets->presentModes != NULL) free(swapDets->presentModes);
		// }
	}
	devDets->phyDev = deviceList[bestIndex];
	vkGetPhysicalDeviceProperties(deviceList[bestIndex], &props);//we could just save the best but whatever
	// SwapChainSupportDetails* tmpSupDets;
	// QueueFamilyIndex* tmpFam;
	IsDeviceCompatible(deviceList[bestIndex],surface,props,&devDets->families,swapDets);
	// devDets->swapSupport = *tmpSupDets;
	// devDets->families = *tmpFam;
	// free(tmpSupDets);
	// free(tmpFam);
	devDets->phyProps = props;
	switch(devDets->phyProps.apiVersion){
		case VK_API_VERSION_1_1: printf("Disclaimer: using Vulkan version 1.1!\n");
		case VK_API_VERSION_1_0: printf("Disclaimer: using Vulkan version 1.0!\n");
	}
	// if (devDets->phyDev == NULL) {
	// 	Error("There are no devices that match our requirements!\n");
	// 	return 0;
	// }
	return 1;
}

//creates the CreateInfo for the queue families
//create the device info
//create the device
//input instance,input surface,output phyDevice,output families, output device
Device::Device(VkSurfaceKHR surface,SwapChainSupportDetails* swapDets) {
	VkInstance instance = GetCurrentInstance()->instance;
	if (!GetPhysicalDevice(instance,surface,this,swapDets)) {
		return;
	}
	//Creating a graphics logical device-------------------------------------
	//create info about queues to be created
	float priorities = 1.0f;//everything might just be 1

	VkDeviceQueueCreateInfo* queueInfos = (VkDeviceQueueCreateInfo*)malloc(sizeof(VkDeviceQueueCreateInfo) * families.familyCount);
	unsigned int nextIndex = 0;
	if (families.exists & GRAPHICS_BIT) {
		queueInfos[nextIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfos[nextIndex].queueFamilyIndex = families.graphics;
		queueInfos[nextIndex].queueCount = 1;
		queueInfos[nextIndex].pQueuePriorities = &priorities;//this can be an array in the future
		queueInfos[nextIndex].pNext = NULL;
		queueInfos[nextIndex].flags = 0;
		nextIndex++;
	}
	//-------------------------
	if (families.exists & PRESENTATION_BIT && families.graphics != families.presentation) {
		queueInfos[nextIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfos[nextIndex].queueFamilyIndex = families.presentation;
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
	deviceInfo.queueCreateInfoCount = families.familyCount;//when we add more queues we need to inc this
	deviceInfo.pQueueCreateInfos = queueInfos;
	deviceInfo.flags = 0;
	deviceInfo.pNext = NULL;
	if (vkCreateDevice(phyDev, &deviceInfo, NULL, &(device)) != VK_SUCCESS) {
		Error("Couldn't create a logical device. oh no\n");
	}


	vkGetDeviceQueue(device, families.graphics, 0, &(queues[0]));//3rd argument is the 0th queue of the queue family
	vkGetDeviceQueue(device, families.presentation, 0, &(queues[1]));
	// //create the swapchain for the device
	// if(!CreateSwapChain(dets->device,&(dets->families),surface,&(dets->swapChain))){
	// 	return 0;
	// }	

}

Device::Device(){
	VkInstance instance = GetCurrentInstance()->instance;
	glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
	GLFWwindow* dummy = glfwCreateWindow(WIDTH,HEIGHT,"Dummy",NULL,NULL);
	glfwWindowHint(GLFW_VISIBLE,GLFW_TRUE);
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(GetCurrentInstance()->instance, dummy, NULL, &surface)!=VK_SUCCESS) {
		Error("Couldn't create a surface!\n");
	}
	SwapChainSupportDetails swapDets;
	// if(!DeviceGetSwapChainDetails(phyDev, surface, &swapDets)){
    //     Error("This surface doesn't comply with the picked device. wack\n");
    // }

	if (!GetPhysicalDevice(instance,surface,this,&swapDets)) {
		return;
	}
	//Creating a graphics logical device-------------------------------------
	//create info about queues to be created
	float priorities = 1.0f;//everything might just be 1

	VkDeviceQueueCreateInfo* queueInfos = (VkDeviceQueueCreateInfo*)malloc(sizeof(VkDeviceQueueCreateInfo) * families.familyCount);
	unsigned int nextIndex = 0;
	if (families.exists & GRAPHICS_BIT) {
		queueInfos[nextIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfos[nextIndex].queueFamilyIndex = families.graphics;
		queueInfos[nextIndex].queueCount = 1;
		queueInfos[nextIndex].pQueuePriorities = &priorities;//this can be an array in the future
		queueInfos[nextIndex].pNext = NULL;
		queueInfos[nextIndex].flags = 0;
		nextIndex++;
	}
	//-------------------------
	if (families.exists & PRESENTATION_BIT && families.graphics != families.presentation) {
		queueInfos[nextIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfos[nextIndex].queueFamilyIndex = families.presentation;
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
	deviceInfo.queueCreateInfoCount = families.familyCount;//when we add more queues we need to inc this
	deviceInfo.pQueueCreateInfos = queueInfos;
	deviceInfo.flags = 0;
	deviceInfo.pNext = NULL;
	if (vkCreateDevice(phyDev, &deviceInfo, NULL, &(device)) != VK_SUCCESS) {
		Error("Couldn't create a logical device. oh no\n");
	}


	vkGetDeviceQueue(device, families.graphics, 0, &(queues[0]));//3rd argument is the 0th queue of the queue family
	vkGetDeviceQueue(device, families.presentation, 0, &(queues[1]));
	vkDestroySurfaceKHR(instance,surface,NULL);
    //destroy GLFWwindow
    glfwDestroyWindow(dummy);
}