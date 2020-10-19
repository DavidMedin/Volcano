#include "source.h"
Device* device;
Window* window;
Window* otherWindow;
Shader* shad;

void Shutdown() {
	vkDeviceWaitIdle(device->device);//finish everything it is doing so the semaphores are no longer in use

	DestoryWindow(device,window);
	DestoryWindow(device,otherWindow);
	delete shad;
	vkDestroyDevice(device->device,NULL);
	GetCurrentInstance()->~Instance();
	SetCurrentInstance(nullptr);
	DestroyGLFW();
}

int main() {
	InitVolcano();
	device = new Device();

	ShaderGroup group;
	group.index = 0;
	shad = new Shader(device,&group,"Volcano/src/shaders/vertex.spv","Volcano/src/shaders/fragment.spv");


	window = new Window("TestWindow",device);
	otherWindow = new Window("otherWindow", device);

	//game loop
	double afterTime = 0;
	double beforeTime = 0;
	double dt = 0;
	while (!glfwWindowShouldClose(window->window)) {
		//get delta time
		afterTime = glfwGetTime();
		dt = afterTime - beforeTime;
		beforeTime = afterTime;

		shad->DrawFrame(window->swapchain);
		shad->DrawFrame(otherWindow->swapchain);
		glfwPollEvents();
	}
	Shutdown();
	return 0;
}
