#include "validationLayers.h"

//check all available validation layers
int CheckValidationLayerSupport() {
	uint32_t availableLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL);
	VkLayerProperties* availableLayers = malloc(sizeof(VkLayerProperties) * availableLayerCount);

	vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);


	unsigned int layerCount = 0;
	const char** layers = GetInstanceValidationLayers(&layerCount);
	//go through requested layers and compare to available layers
	if (availableLayers != NULL && availableLayerCount > 0) {
		//i iterates through requested layers
		for (unsigned int i = 0; i < layerCount; i++) {//validationlayers is just an array of char*
			int found = 0;
			//y iterates through available layers
			for (unsigned int y = 0; y < availableLayerCount; y++) {
				//check to see if i exists in y
				if (!strcmp(layers[i], availableLayers[y].layerName)) {
					//if it does, stop searching
					found = 1;
					break;
				}
			}
			//if it wasn't found, panic
			if (!found) {
				printf("validation layer \"%s\" is not an available validation layer!\n", layers[i]);
				free(availableLayers);
				return 1;
			}
		}
		//if you've made it this far, you got all your shit together
		free(availableLayers);
		return 0;
	}
	else {
		printf("no available validation layers. idk man, kinda broken\n");
		free(availableLayers);
		return 1;
	}

}

//define a validation callback (for debug info)
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	printf("Validation Layer: %s \n", pCallbackData->pMessage);
	return VK_FALSE;
}

//create a debug callback
VkResult CreateDebugMesseger(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger) {
	//because vkCreateDebugUtilsMessengerEXT is an extention function it isn't loaded for some stupid reason. we have to query it I guess
	// also no idea why we need to give it instance, maybe different instances hold different extentions so you need a check or something
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != NULL) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
//destroy a debug callback
void DestroyDebugMessenger(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator) {
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL) {
		func(instance, debugMessenger, pAllocator);
	}
}

void CreateDebugInfo(VkDebugUtilsMessengerCreateInfoEXT* debugInfo) {
	debugInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;//this struct NEEDS to have this define as its type
	debugInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; //defines the severity of the message this function will accept
	debugInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT; // defines	the mesage types this functions will accept
	debugInfo->pfnUserCallback = DebugCallback;//define the function that we are to give to Vulkan
	debugInfo->pUserData = NULL;
	debugInfo->pNext = NULL;
	debugInfo->flags = 0;

}