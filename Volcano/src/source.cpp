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


	InitVolcano();
	device = new Device();

	ShaderGroup group;//will contain description of renderpass in the future
	group.index = 0;
	shad = new Shader(device,&group,"Volcano/src/shaders/vertex.spv","Volcano/src/shaders/fragment.spv");
	

	VertexBuffer* buff = CreateVertexBuffer(shad,3,0,0,BufferRate::PER_VERTEX,&Test,&Test.pos,&Test.color);
	
	std::vector<Test_t> dataVec = {
		{{0,0.5f},{1,0,0}},
		{{0.5f,-0.5f},{0,1,0}},
		{{-0.5f,-0.5f},{0,0,1}}
	};
	void* data;
	buff->MapData(&data);
	memcpy(data,dataVec.data(),sizeof(Test_t)*3);
	buff->UnMapData();

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
