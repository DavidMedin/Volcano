#include "window.h"
#include "shader.h"
std::list<Window*> windows;
extern std::list<Shader*> shadList;
extern std::list<Device*> deviceList;


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
    SetCurrentDevice(new Device());
}

void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    Window* win = (Window*)glfwGetWindowUserPointer(window);
    win->swapchain->windowResized = true;
}

Window::Window(const char* windowName){
    glfwWindowHint(GLFW_RESIZABLE,GLFW_TRUE);
    window = glfwCreateWindow(WIDTH,HEIGHT,windowName,NULL,NULL);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	instance = GetCurrentInstance();
    
    if (glfwCreateWindowSurface(GetCurrentInstance()->instance, window, NULL, &surface)!=VK_SUCCESS) {
		Error("Couldn't create a surface!\n");
	}
    swapchain = new SwapChain(GetCurrentDevice(),surface);
    swapchain->win = window;
    // CreateSwapChain((*device),surface,&swapchain);
    vulkanInit = 1;
    windows.push_back(this);
}

void DestroyWindow(Device* device, Window* window){
    Instance* instance = window->instance;
    //destory everything in DeviceDetails
    
    delete window->swapchain;
    // vkDestroyDevice(device->device,NULL);
    //destroy VkSurfaceKHR
    vkDestroySurfaceKHR(instance->instance,window->surface,NULL);
    //destroy GLFWwindow
    glfwDestroyWindow(window->window);
}

void DestroyVolcano(){
    for(auto device: deviceList){
        vkDeviceWaitIdle(device->device);
        for(auto win: windows){
            DestroyWindow(device,win);
        }
        for(auto shad : shadList){
            delete shad;
        }
        delete device;
    }
    delete GetCurrentInstance();
    SetCurrentInstance(nullptr);
    DestroyGLFW();
}