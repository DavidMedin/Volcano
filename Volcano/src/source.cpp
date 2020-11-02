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

struct Test{
	int hi;
	glm::vec2 bye;
};

int main() {



	InitVolcano();
	device = new Device();

	//Create vertex data
	Test data;
	data.hi = 1;
	data.bye = glm::vec2(1,2);
	CreateVertexBuffer<Test,int,glm::vec2>(&data,{&data.hi,&data.bye},0,0,BufferRate::PER_VERTEX);


	ShaderGroup group;//will contain description of renderpass in the future
	group.index = 0;
	shad = new Shader(device,&group,"Volcano/src/shaders/vertex.spv","Volcano/src/shaders/fragment.spv");


	window = new Window("TestWindow",device);
	otherWindow = new Window("otherWindow", device);


	//game loop
	double afterTime = 0;
	double beforeTime = 0;
	double dt = 0;
	while (!glfwWindowShouldClose(window->window) && !glfwWindowShouldClose(otherWindow->window)) {
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
