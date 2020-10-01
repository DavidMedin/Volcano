#include "source.h"
Device* device;
Window* window;
Shader* shad;
std::shared_ptr<RenderPass> renderPass;

void Shutdown() {
	vkDeviceWaitIdle(device->device);//finish everything it is doing so the semaphores are no longer in use

	// vkDestroyRenderPass(device->device,renderPass,NULL);
	DestroyRenderpasses();
	delete shad;
	DestoryWindow(device,window);
	GetCurrentInstance()->~Instance();
	SetCurrentInstance(nullptr);
	DestroyGLFW();
}

int main() {
	InitVolcano();
	device = new Device();
	window = new Window("TestWindow",device);

	renderPass = GetRenderpass(window->swapchain->GetFormat(),device,0);
	window->swapchain->RegisterRenderPasses({renderPass});

	shad = new Shader(device,0,window->swapchain,"Volcano/src/shaders/vertex.spv","Volcano/src/shaders/fragment.spv");
	shad->RegisterSwapChains({window->swapchain});

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
		glfwPollEvents();
	}
	Shutdown();
	return 0;
}
