#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

const char* validationLayers[] = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
	const int enableValidationLayers = 0;
#else
	const int enableValidationLayers = 1;
#endif

GLFWwindow* window;
VkInstance instance;


//check all available validation layers
int CheckValidationLayerSupport() {
	uint32_t availableLayerCount=0;
	vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL);
	VkLayerProperties* availableLayers = malloc(sizeof(VkLayerProperties)*availableLayerCount);

	vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);


	//go through requested layers and compare to available layers
	if(availableLayers!=NULL && availableLayerCount > 0){
		//i iterates through requested layers
		for (int i = 0; i < sizeof(validationLayers)/sizeof(char*); i++) {//validationlayers is just an array of char*
			int found=0;
			//y iterates through available layers
			for (int y = 0; y < availableLayerCount; y++) {
				//check to see if i exists in y
				if (!strcmp(validationLayers[i], availableLayers[y].layerName)) {
					//if it does, stop searching
					found = 1;
					break;
				}
			}
			//if it wasn't found, panic
			if (!found) {
				printf("validation layer \"%s\" is not an available validation layer!\n", validationLayers[i]);
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
void GetRequiredExtensions(unsigned int* count, char*** names) {//pointer to a pointer to arrays(pointers)
	char** list;//points to an array of strings
	unsigned int listCount = 0;

	//add glfw required extentions to list
	list = glfwGetRequiredInstanceExtensions(&listCount);//fill list and get count
	
	if (enableValidationLayers) {
		//add validation layer extention to list---------------------------------
		//need to realocate list
		listCount++;
		char** tmpList = malloc(sizeof(char*) * listCount);
		memcpy(tmpList, list, (listCount - 1)*sizeof(char*));//copy the addresses from list to tmplist using count+1-1
		//free(list);
		tmpList[listCount - 1] = "VK_EXT_debug_utils";
		list = tmpList;
	}
	*count = listCount;
	*names = list;
}

int main() {

	if (!glfwInit()) {
		printf("panic\n");
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(800, 600, "TestWindow", NULL , NULL);

	//-----------init vulkan-----------------------------
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "TestWindow";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "PineGL";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;
	appInfo.pNext = NULL;

	VkInstanceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.pNext = NULL;
	//bop the extensions in
	unsigned int extentionCount = 0;
	char** extentions=NULL;
	GetRequiredExtensions(&extentionCount, &extentions);

	createInfo.enabledExtensionCount = extentionCount;
	createInfo.ppEnabledExtensionNames = extentions;

	//enable validation layers
	if (enableValidationLayers && CheckValidationLayerSupport()) {
		printf("Missing validation layers!\n");
		createInfo.enabledLayerCount = 0;
	}
	else {
		createInfo.enabledLayerCount = sizeof(validationLayers) / sizeof(char*);
		createInfo.ppEnabledLayerNames = validationLayers;
	}
	//create the Vulkan Instance
	if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
		printf("Couldn't create a vulkan instance!\n");
		//create an error
	}
	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();
	}
	vkDestroyInstance(instance, NULL);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
