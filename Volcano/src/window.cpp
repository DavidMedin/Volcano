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
void InitVolcano(){
    if(!vulkanInit){
        if(!InitGLFW()) return;
        Instance* instance = new Instance();
        SetCurrentInstance(instance);
    }
}

void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    Window* win = (Window*)glfwGetWindowUserPointer(window);
    win->swapchain->windowResized = true;
}

Window::Window(const char* windowName,Device* device){
    glfwWindowHint(GLFW_RESIZABLE,GLFW_TRUE);
    window = glfwCreateWindow(WIDTH,HEIGHT,windowName,NULL,NULL);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	instance = GetCurrentInstance();
    
    if (glfwCreateWindowSurface(GetCurrentInstance()->instance, window, NULL, &surface)!=VK_SUCCESS) {
		Error("Couldn't create a surface!\n");
	}
    swapchain = new SwapChain(device,surface);
    swapchain->win = window;
    // CreateSwapChain((*device),surface,&swapchain);
    vulkanInit = 1;
}

void DestoryWindow(Device* device, Window* window){
    Instance* instance = window->instance;
    //destory everything in DeviceDetails
    for(unsigned int i = 0;i < window->swapchain->imageCount;i++){
        vkDestroyImageView(device->device,window->swapchain->imageViews[i],NULL);
    }
    delete window->swapchain;
    vkDestroyDevice(device->device,NULL);
    //destroy VkSurfaceKHR
    vkDestroySurfaceKHR(instance->instance,window->surface,NULL);
    //destroy GLFWwindow
    glfwDestroyWindow(window->window);
}