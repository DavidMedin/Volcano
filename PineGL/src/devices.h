#pragma once
#include "globalVulkan.h"
#include "instance.h"

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

typedef struct PhysicalDeviceDetails{
	VkPhysicalDevice phyDev;
	QueueFamilyIndex families;
	SwapChainSupportDetails swapDetails;
}PhysicalDeviceDetails;

int DeviceGetSwapChainDetails(VkPhysicalDevice device, VkSurfaceKHR surface, SwapChainSupportDetails* details);
int HasRequiredQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, QueueFamilyIndex* queueFamilies);
int DeviceHasRequiredExtentions(VkPhysicalDevice device, char** requestedExtentions);
int GetPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* phyDevice, QueueFamilyIndex* queueFamilies);
int CreateDevices(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* phyDevice, QueueFamilyIndex* queueFamilies, VkDevice* device);
