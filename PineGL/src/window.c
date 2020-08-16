#include "window.h"

int doneInit = 0;

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

void CreateWindow(const char* windowName,Instance* instance, Window* pWindow){
	if(!InitGLFW()) return;
	*pWindow = malloc(sizeof(struct Window));
    (*pWindow)->window = glfwCreateWindow(WIDTH,HEIGHT,windowName,NULL,NULL);
    //create the instance
    CreateInstance(windowName,instance);
	if (glfwCreateWindowSurface((*instance)->instance, (*pWindow)->window, NULL, &((*pWindow)->surface))!=VK_SUCCESS) {
		Error("Couldn't create a surface!\n");
	}
}

void DestoryWindow(Instance instance, Window window){

    //destory everything in DeviceDetails
    for(unsigned int i = 0;i < window->devDets.swapChain.imageCount;i++){
        vkDestroyImageView(window->devDets.device,window->devDets.swapChain.imageViews[i],NULL);
    }
	vkDestroySwapchainKHR(window->devDets.device,window->devDets.swapChain.swapChain,NULL);
    vkDestroyDevice(window->devDets.device,NULL);
    //destroy VkSurfaceKHR
    vkDestroySurfaceKHR(instance->instance,window->surface,NULL);
    //destroy GLFWwindow
    glfwDestroyWindow(window->window);
}