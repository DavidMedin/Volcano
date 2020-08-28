#pragma once
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "globalVulkan.h"
#include "instance.h"
#include "errorCatch.h"
#include "graphics.h"
// #include "swapchain.h"


typedef struct Window{
	GLFWwindow* window;
    VkSurfaceKHR surface;
	SwapChain swapchain;
	// DeviceDetails devDets;
}*Window;
int InitGLFW();
void DestroyGLFW();
void CreateWindow(const char* windowName,Instance* instance,Device* device, Window* pWindow);
void DestoryWindow(Instance instance,Device device, Window window);
//void shutdownwindow