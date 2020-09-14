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


struct Window{
	Instance* instance;//the instance that created it. Only really used for destruction of the window.

	GLFWwindow* window;
    VkSurfaceKHR surface;
	SwapChain* swapchain;
	// DeviceDetails devDets;
	Window(const char* windowName,Device** device);
};
int InitGLFW();
void DestroyGLFW();
// void CreateWindow(const char* windowName,Instance* instance,Device* device, Window* pWindow);
void DestoryWindow(Device* device, Window *window);
//void shutdownwindow