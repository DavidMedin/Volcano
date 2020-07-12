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

//our globals --- declared in globalVulkan.h






GLFWwindow* window;
VkInstance instance;
VkDebugUtilsMessengerEXT debugMessenger;//this contains the 'instance' of the debug messenger

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;//we'll do mutiple of these in the future
VkDevice device = VK_NULL_HANDLE;

//queues
VkQueue graphicsQueue = VK_NULL_HANDLE;
VkQueue presentQueue = VK_NULL_HANDLE;

VkSurfaceKHR surface;

void Shutdown() {
	if (enableValidationLayers) {
		DestroyDebugMessenger(instance, debugMessenger, NULL);
	}
	vkDestroyDevice(device, NULL);
	vkDestroySurfaceKHR(instance, surface, NULL);
	vkDestroyInstance(instance, NULL);
	glfwDestroyWindow(window);
	glfwTerminate();

}

//btw vulkan types and defines are way to fucking long
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
	QueueFamilyIndex queueFamilies;
	if (!CreateDevices(instance,surface,&physicalDevice,&queueFamilies,&device)) {
		printf("Couldn't create devices!\n");
	}
	//create the queues for the device (graphics calls and presentation calls)
	vkGetDeviceQueue(device, queueFamilies.graphics, 0, &graphicsQueue);//3rd argument is the 0th queue of the queue family
	vkGetDeviceQueue(device, queueFamilies.presentation, 0, &presentQueue);

	//game loop
	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();
	}
	Shutdown();
	return 0;
}
