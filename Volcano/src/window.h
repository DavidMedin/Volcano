#pragma once
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "globalVulkan.h"
#include "devices.h"
#include "instance.h"
#include "errorCatch.h"

typedef struct Window{
	GLFWwindow* window;
    VkSurfaceKHR surface;
	DeviceDetails devDets;
}*Window;
int InitGLFW();
void DestroyGLFW();
void CreateWindow(const char* windowName,Instance* instance,Window* pWindow);
void DestoryWindow(Instance instance, Window window);
//void shutdownwindow