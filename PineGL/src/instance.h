#pragma once
#include "vulkan/vulkan.h"
#include <stdlib.h>

#include "errorCatch.h"
#include "globalVulkan.h"
#include "validationLayers.h"
typedef struct Instance{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
}*Instance;

//init glfw

void CreateInstance(const char* appName, Instance* instance);
void DestoryInstance(Instance instance);

//not used outside of instance.c
const char** GetInstanceValidationLayers(unsigned int* count);
void InstanceGetRequiredExtensions(unsigned int* count, char*** names);
void InstantiateInstance(VkInstance* instance, VkDebugUtilsMessengerEXT* debugMessenger);
//void Shutdown();