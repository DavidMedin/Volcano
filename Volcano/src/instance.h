#pragma once
#include "vulkan/vulkan.h"
#include <stdlib.h>

#include "errorCatch.h"
#include "globalVulkan.h"
#include "validationLayers.h"
#include "devices.h"
struct Instance{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    Instance();
    ~Instance();
};

//init glfw
// void StartVulkan(Instance* instance,DeviceDetails* device);
// void CreateInstance(Instance* instance);
// void DestoryInstance(Instance* instance);

//not used outside of instance.c
const char** GetInstanceValidationLayers(unsigned int* count);
void InstanceGetRequiredExtensions(unsigned int* count, char*** names);
void InstantiateInstance(VkInstance* instance, VkDebugUtilsMessengerEXT* debugMessenger);

Instance* GetCurrentInstance();
void SetCurrentInstance(Instance* instance);
//void Shutdown();