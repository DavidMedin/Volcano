#include "window.h"

int doneInit = 0;
int vulkanInit = 0;
int InitGLFW() {
    if(!doneInit){
        if (!glfwInit()) {
            Error("Couldn't initialize glfw!\n");
            return 0;
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        doneInit = 1;
        return 1;
    }
    return 1;
}
void DestroyGLFW(){
    if(doneInit){
        glfwTerminate();
        doneInit = 0;
    }
}
/*--first time input:
    uninitialized instance
    uninitialized device
    uninitialized window
--fist time Output:
    instance
    device
    pWindow
--not first time Input:
    windowName
    device
    instance
    uninitialized window
--not first time output:
    pwindow
*/
void CreateWindow(const char* windowName,Instance* instance,DeviceDetails* device, Window* pWindow){
	//once
    if(!vulkanInit)
        if(!InitGLFW()) return;

	*pWindow = malloc(sizeof(struct Window));
    (*pWindow)->window = glfwCreateWindow(WIDTH,HEIGHT,windowName,NULL,NULL);

    //once
    if(!vulkanInit)
        CreateInstance(instance);
	
    
    if (glfwCreateWindowSurface((*instance)->instance, (*pWindow)->window, NULL, &((*pWindow)->surface))!=VK_SUCCESS) {
		Error("Couldn't create a surface!\n");
	}
    if(!vulkanInit)
        CreateDevices((*instance)->instance,(*pWindow)->surface,device);
    else
        if(!IsDeviceCompatible(device->phyDev,(*pWindow)->surface,device->phyProps,device)){
            Error("This surface doesn't comply with the picked device. wack\n");
        }
    CreateSwapChain(device,(*pWindow)->surface,&(*pWindow)->swapchain);
    vulkanInit = 1;
}

void DestoryWindow(Instance instance,DeviceDetails device, Window window){

    //destory everything in DeviceDetails
    for(unsigned int i = 0;i < window->swapchain.imageCount;i++){
        vkDestroyImageView(device.device,window->swapchain.imageViews[i],NULL);
    }
	vkDestroySwapchainKHR(device.device,window->swapchain.swapChain,NULL);
    vkDestroyDevice(device.device,NULL);
    //destroy VkSurfaceKHR
    vkDestroySurfaceKHR(instance->instance,window->surface,NULL);
    //destroy GLFWwindow
    glfwDestroyWindow(window->window);
}