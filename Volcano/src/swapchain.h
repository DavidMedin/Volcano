#pragma once
#include "globalVulkan.h"
#include "devices.h"

typedef struct SwapChain{
	VkSwapchainKHR swapChain;

	//indices into swapDets.formats and presentModes
	SwapChainSupportDetails* swapDets;
	VkExtent2D swapExtent;
	unsigned int chosenFormat;//use .format
	unsigned int chosenPresent;
	//images in the swapchain
	unsigned int imageCount;//count for both images and imageViews
	VkImage* images;//array of handles to VkImages
	VkImageView* imageViews;

	DeviceDetails* device;
}SwapChain;

int CreateSwapChain(DeviceDetails* device,VkSurfaceKHR surface, SwapChain* swapChain);
