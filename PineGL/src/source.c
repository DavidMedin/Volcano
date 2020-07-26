#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

//our stuff
#include "globalVulkan.h"
#include "validationLayers.h"
#include "devices.h"
#include "instance.h"
#include "GraphicsPipeline.h"
//our globals --- declared in globalVulkan.h






GLFWwindow* window;
VkInstance instance;
VkDebugUtilsMessengerEXT debugMessenger;//this contains the 'instance' of the debug messenger

// VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;//we'll do mutiple of these in the future
// VkDevice device = VK_NULL_HANDLE;
// //queues
// VkQueue graphicsQueue = VK_NULL_HANDLE;
// VkQueue presentQueue = VK_NULL_HANDLE;
// //swapchain
// VkSwapchainKHR swap = VK_NULL_HANDLE;

DeviceDetails devDets;

VkSurfaceKHR surface;

void Shutdown() {
	for(unsigned int i = 0; i < devDets.swapChain.imageCount;i++){
		vkDestroyImageView(devDets.device,devDets.swapChain.imageViews[i],NULL);
	}
	vkDestroySwapchainKHR(devDets.device,devDets.swapChain.swapChain,NULL);
	vkDestroyDevice(devDets.device, NULL);
	vkDestroySurfaceKHR(instance, surface, NULL);
	if (enableValidationLayers) {
		DestroyDebugMessenger(instance, debugMessenger, NULL);
	}
	vkDestroyInstance(instance, NULL);
	glfwDestroyWindow(window);
	glfwTerminate();

}

//btw vulkan types and defines are way too fucking long
int main() {

	//init glfw
	if (!InitGLFW(&window)) {
		printf("Panic! Glfw didn't initialize!\n");
	}

	//create vulkan instance
	InstantiateInstance(&instance,&debugMessenger);

	//get surface from windows
	if (glfwCreateWindowSurface(instance, window, NULL, &surface)!=VK_SUCCESS) {
		printf("Couldn't create a surface!\n");
	}

	//find the best device (GPU in this case) and return a virtual device, physical device, and the queue families available  for that device
	if (!CreateDevices(instance,surface,&devDets)) {
		printf("Couldn't create devices!\n");
	}
	//left this out of create devices to make it more dynamic
	//create the queues for the device (graphics calls and presentation calls)
	vkGetDeviceQueue(devDets.device, devDets.families.graphics, 0, &(devDets.queues[0]));//3rd argument is the 0th queue of the queue family
	vkGetDeviceQueue(devDets.device, devDets.families.presentation, 0, &(devDets.queues[1]));

	CreateGraphicsPipeline();

	//game loop
	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();
	}
	Shutdown();
	return 0;
}
