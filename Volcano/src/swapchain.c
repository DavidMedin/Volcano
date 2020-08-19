#include "swapchain.h"


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

int CreateSwapChain(DeviceDetails* devDets,VkSurfaceKHR surface, SwapChain* swapChain){
	//shortcuts
	VkDevice device = devDets->device;
	QueueFamilyIndex* famz = &devDets->families;

	swapChain->swapDets = &devDets->swapSupport;
	swapChain->chosenFormat = Swap_GetBestSurfaceFormat(swapChain->swapDets);
	swapChain->chosenPresent = Swap_GetBestPresentMode(swapChain->swapDets);
	VkSurfaceFormatKHR* format = &(swapChain->swapDets->formats[swapChain->chosenFormat]);
	VkPresentModeKHR present = swapChain->swapDets->presentModes[swapChain->chosenPresent];
	VkExtent2D extent = Swap_GetBestSwapExtent(swapChain->swapDets);

	unsigned int imageCount = swapChain->swapDets->capabilities.minImageCount + 1;
	//if maximage is 0, unlimited images; otherwise get the min of the two
	if (swapChain->swapDets->capabilities.maxImageCount > 0 && imageCount > swapChain->swapDets->capabilities.maxImageCount) {
		imageCount = swapChain->swapDets->capabilities.maxImageCount;
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
		if(famz->presentation == famz->graphics){
		swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//only one family can push to the swapchain
		swapInfo.queueFamilyIndexCount = 0;
		swapInfo.pQueueFamilyIndices = NULL;
	}else{
		swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//more than one can push, need to juggle permissions tho
		swapInfo.queueFamilyIndexCount = 2;
		unsigned int indices[] = {famz->graphics,famz->presentation};
		swapInfo.pQueueFamilyIndices = indices;
	}

	swapInfo.preTransform = swapChain->swapDets->capabilities.currentTransform; // no applied transform
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

	if(!CreateImageViews(device,swapChain->imageCount,swapChain->images,&swapChain->swapDets->formats[swapChain->chosenFormat],&swapChain->imageViews)){
		Error("Couldn't create image views!\n");
		return 0;
	}
	swapChain->swapExtent = extent;
	return 1;
}
