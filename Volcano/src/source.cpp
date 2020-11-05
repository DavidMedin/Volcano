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

struct Test_t{
	glm::vec2 pos;
	glm::vec3 color;
}Test;

int main() {

	/*
	trello isn't working so I am writing here
	-shader vertexbuffer vector to list
	-list of shaders a vertexbuffer is in (in vertexbuffer struct)
	-destroyVolcano to dealloc all volcano objects, and destroy all vulkan objects
		-need a list of everything somehow
	*/

	InitVolcano();
	device = new Device();

	// //Create vertex data
	// Test data[2] ={ {1,{1,2} },
	// 			    {2,{2,3} }};
	// printf("%d\n",sizeof(*((Test*)data)));


	ShaderGroup group;//will contain description of renderpass in the future
	group.index = 0;
	shad = new Shader(device,&group,"Volcano/src/shaders/vertex.spv","Volcano/src/shaders/fragment.spv");

	CreateVertexBuffer(shad,0,0,BufferRate::PER_VERTEX,&Test,&Test.pos,&Test.color);

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
