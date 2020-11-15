#include "source.h"
Device* device;
Window* window;
Window* otherWindow;
Shader* shad;

void Shutdown() {
	vkDeviceWaitIdle(device->device);//finish everything it is doing so the semaphores are no longer in use

	DestoryWindow(device,window);
	// DestoryWindow(device,otherWindow);

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


	InitVolcano();
	device = new Device();

	ShaderGroup group;//will contain description of renderpass in the future
	group.index = 0;
	shad = new Shader(device,&group,"Volcano/src/shaders/vertex.spv","Volcano/src/shaders/fragment.spv");


	VertexBuffer* buff = CreateVertexBuffer(shad,3,0,0,BufferRate::PER_VERTEX,&Test,&Test.pos,&Test.color);

	std::vector<glm::vec2> pos = {{0.0f, -0.5f},{0.5f, 0.5f},{-0.5f, 0.5f}};
	std::vector<glm::vec3> color = {{1.0f, 0.0f, 0.0f},{0.0f, 1.0f, 0.0f},{1.0f, 0.0f, 1.0f}};
	buff->WriteData(0,pos.data(),color.data());


	window = new Window("TestWindow",device);


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
		// shad->DrawFrame(otherWindow->swapchain);
		glfwPollEvents();
	}
	Shutdown();
	return 0;
}
