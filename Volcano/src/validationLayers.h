#pragma once


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

//our stuff
#include "errorCatch.h"
#include "globalVulkan.h"
#include "instance.h"

int CheckValidationLayerSupport();
VkResult CreateDebugMesseger(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugMessenger(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator);
void CreateDebugInfo(VkDebugUtilsMessengerCreateInfoEXT* debugInfo);