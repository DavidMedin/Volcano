#pragma once
#include "globalVulkan.h"

//init glfw
int InitGLFW(GLFWwindow** window);

const char** GetInstanceValidationLayers(unsigned int* count);
void InstanceGetRequiredExtensions(unsigned int* count, char*** names);
void InstantiateInstance(VkInstance* instance, VkDebugUtilsMessengerEXT* debugMessenger);
//void Shutdown();