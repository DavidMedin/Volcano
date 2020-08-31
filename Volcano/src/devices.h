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

typedef struct Device{
	VkPhysicalDevice phyDev;
	VkPhysicalDeviceProperties phyProps;

	VkDevice device;
	SwapChainSupportDetails swapSupport;

	VkQueue queues[2];//[0] = graphics, [1] = presentation
	// unsigned int queueCount;
	QueueFamilyIndex families;
}Device;

int CreateDevices(VkInstance instance,VkSurfaceKHR surface, Device* dets);
int IsDeviceCompatible(VkPhysicalDevice phyDev,VkSurfaceKHR surface,VkPhysicalDeviceProperties props,QueueFamilyIndex* fams, SwapChainSupportDetails* swapDets);