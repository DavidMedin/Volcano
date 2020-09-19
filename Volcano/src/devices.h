#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "globalVulkan.h"
#include "instance.h"
#include "errorCatch.h"

#include <math.h>
#include <stdint.h>
typedef enum QueueFamilyBit {
	NONE_BIT = 0b0,
	GRAPHICS_BIT = 0b1,//should define a binary number
	PRESENTATION_BIT = 0b10
}QueueFamilyBit;
struct QueueFamilyIndex {
	unsigned int exists;//use exists & GRAPHICS_BIT to get/set it
	unsigned int graphics;
	unsigned int presentation;//can be the same as graphics

	//fill this with more stuff when we need to add more queues

	unsigned int familyCount;//won't include a dup presentation
};

//free formats and presentModes after use
struct SwapChainSupportDetails { 
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR* formats;
	unsigned int formatCount;
	VkPresentModeKHR* presentModes;
	unsigned int presentCount;
};

struct Device{
	VkPhysicalDevice phyDev;
	VkPhysicalDeviceProperties phyProps;

	VkDevice device;
	SwapChainSupportDetails swapSupport;

	VkQueue queues[2];//[0] = graphics, [1] = presentation
	// unsigned int queueCount;
	QueueFamilyIndex families;
	Device(VkSurfaceKHR surface);
	Device();
};

// int CreateDevice(VkInstance instance,VkSurfaceKHR surface, Device* dets);
int IsDeviceCompatible(VkPhysicalDevice phyDev,VkSurfaceKHR surface,VkPhysicalDeviceProperties props,QueueFamilyIndex* fams, SwapChainSupportDetails* swapDets);