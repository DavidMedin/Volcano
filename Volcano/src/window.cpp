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
Window::Window(const char* windowName,Device** device){
	//once
    if(!vulkanInit)
        if(!InitGLFW()) return;

	// *pWindow = (Window *)malloc(sizeof(struct Window));
    window = glfwCreateWindow(WIDTH,HEIGHT,windowName,NULL,NULL);

    //once
    if(!vulkanInit){
        Instance* instance = new Instance();
        SetCurrentInstance(instance);
    }
	instance = GetCurrentInstance();
    
    if (glfwCreateWindowSurface(GetCurrentInstance()->instance, window, NULL, &surface)!=VK_SUCCESS) {
		Error("Couldn't create a surface!\n");
	}
    if(!vulkanInit)
        // CreateDevices(instance->instance,surface,device);
        *device = new Device(surface);
    else
        if(!IsDeviceCompatible((*device)->phyDev,surface,(*device)->phyProps,&(*device)->families,&(*device)->swapSupport)){
            Error("This surface doesn't comply with the picked device. wack\n");
        }
    // CreateSwapChain((*device),surface,&swapchain);
    swapchain = new SwapChain((*device),surface);
    vulkanInit = 1;
}

void DestoryWindow(Device* device, Window* window){
    Instance* instance = window->instance;
    //destory everything in DeviceDetails
    for(unsigned int i = 0;i < window->swapchain->imageCount;i++){
        vkDestroyImageView(device->device,window->swapchain->imageViews[i],NULL);
    }
	vkDestroySwapchainKHR(device->device,window->swapchain->swapChain,NULL);
    vkDestroyDevice(device->device,NULL);
    //destroy VkSurfaceKHR
    vkDestroySurfaceKHR(instance->instance,window->surface,NULL);
    //destroy GLFWwindow
    glfwDestroyWindow(window->window);
}