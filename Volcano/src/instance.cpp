#include "instance.h"
Instance* currentInstance;

//requested validation layers
const char* validationLayers[] = {
	"VK_LAYER_KHRONOS_validation"
};

const char** GetInstanceValidationLayers(unsigned int* count) {
	*count = sizeof(validationLayers) / sizeof(char*);
	return validationLayers;
}


Instance::Instance(){
	// *instance = malloc(sizeof(struct Instance));
	InstantiateInstance(&instance,&debugMessenger);
}

Instance::~Instance(){
	if (enableValidationLayers) {
		DestroyDebugMessenger(instance, debugMessenger, NULL);
	}
	vkDestroyInstance(instance, NULL);
}

Instance* GetCurrentInstance(){
	return currentInstance;
}
void SetCurrentInstance(Instance* instance){
	currentInstance = instance;
}

void InstanceGetRequiredExtensions(unsigned int* count, char*** names) {//pointer to a pointer to arrays(pointers)
	char** list;//points to an array of strings
	unsigned int listCount = 0;

	//add glfw required extentions to list
	list = (char**)glfwGetRequiredInstanceExtensions(&listCount);//fill list and get count

	if (enableValidationLayers) {
		//add validation layer extention to list---------------------------------
		//need to realocate list
		listCount++;
		char** tmpList = (char**)malloc(sizeof(char*) * listCount);
		memcpy(tmpList, list, (listCount - 1) * sizeof(char*));//copy the addresses from list to tmplist using count+1-1
		//free(list);
		tmpList[listCount - 1] = "VK_EXT_debug_utils";
		list = tmpList;
	}
	*count = listCount;
	*names = list;
}

void InstantiateInstance(VkInstance* instance, VkDebugUtilsMessengerEXT* debugMessenger) {
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "TestWindow";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Volcano";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;
	appInfo.pNext = NULL;

	//define info about the instance----------------------------------------------------
	VkInstanceCreateInfo instanceInfo;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	//bop the extensions in
	unsigned int extentionCount = 0;
	char** extentions = NULL;
	InstanceGetRequiredExtensions(&extentionCount, &extentions);
	instanceInfo.enabledExtensionCount = extentionCount;
	instanceInfo.ppEnabledExtensionNames = extentions;
	instanceInfo.flags = 0;
	//configure validation layers------------------------------------------------
	VkDebugUtilsMessengerCreateInfoEXT debugInfo;//moved out of scope
	if (enableValidationLayers && CheckValidationLayerSupport()) {
		// Error("Missing validation layers!\n");commented because it is not the lowest level on the call stack (already another error call for this problem)
		instanceInfo.enabledLayerCount = 0;
		instanceInfo.pNext = NULL;
	}
	else {
		instanceInfo.enabledLayerCount = sizeof(validationLayers) / sizeof(char*);
		instanceInfo.ppEnabledLayerNames = validationLayers;
		//populates the debug stuff
		CreateDebugInfo(&debugInfo);
		instanceInfo.pNext = &debugInfo;
	}
	//create the Vulkan Instance--------------------------------------------------------------------
	if (vkCreateInstance(&instanceInfo, NULL, instance) != VK_SUCCESS) {
		Error("Couldn't create a vulkan instance!\n");
		//create an error
	}
	if (enableValidationLayers) {
		//VkDebugUtilsMessengerCreateInfoEXT debugInfo;//moved out of scope
		//CreateDebugInfo(&debugInfo);
		if (CreateDebugMesseger(*instance, &debugInfo, NULL, debugMessenger) != VK_SUCCESS) {
			Error("failed to set up debug messenger!\n");
		}
	}
}
//
//void Shutdown() {
////will only use this if everything is internal
//}
