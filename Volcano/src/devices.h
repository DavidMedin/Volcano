#pragma once
#include "globalVulkan.h"
#include "instance.h"
#include "errorCatch.h"

#include <math.h>
#include <stdint.h>
typedef enum QueueFamilyBit {
	GRAPHICS_BIT = 0b1,//should define a binary number
	PRESENTATION_BIT = 0b10
}QueueFamilyBit;
typedef struct QueueFamilyIndex {
	QueueFamilyBit exists;
	unsigned int graphics;
	unsigned int presentation;//can be the same as graphics

	//fill this with more stuff when we need to add more queues

	unsigned int familyCount;//won't include a dup presentation
}QueueFamilyIndex;

//free formats and presentModes after use
typedef struct SwapChainSupportDetails { 
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR* formats;
	unsigned int formatCount;
	VkPresentModeKHR* presentModes;
	unsigned int presentCount;
}SwapChainSupportDetails;

typedef struct SwapChain{
	VkSwapchainKHR swapChain;

	//indices into swapDets.formats and presentModes
	SwapChainSupportDetails swapDets;
	VkExtent2D swapExtent;
	unsigned int chosenFormat;//use .format
	unsigned int chosenPresent;
	//images in the swapchain
	unsigned int imageCount;//count for both images and imageViews
	VkImage* images;//array of handles to VkImages
	VkImageView* imageViews;
}SwapChain;

typedef struct DeviceDetails{
	VkPhysicalDevice phyDev;
	VkDevice device;
	SwapChain swapChain;

	VkQueue queues[2];//[0] = graphics, [1] = presentation
	// unsigned int queueCount;
	QueueFamilyIndex families;
}DeviceDetails;

int CreateDevices(VkInstance instance,VkSurfaceKHR surface,DeviceDetails* dets);
