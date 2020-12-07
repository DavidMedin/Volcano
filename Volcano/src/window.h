#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <list>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "globalVulkan.h"
#include "instance.h"
#include "errorCatch.h"
 #include "swapchain.h"
//#include "graphics.h"


struct Window{
	Instance* instance;//the instance that created it. Only really used for destruction of the window.

	GLFWwindow* window;
    VkSurfaceKHR surface;
	SwapChain* swapchain;
	Window(const char* windowName);
	void Draw(){
		swapchain->DrawFrame();
	}
};
void InitVolcano();
void DestroyVolcano();
void DestroyWindow(Device* device, Window *window);