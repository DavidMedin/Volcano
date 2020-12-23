#include "source.h"
Window* window;

struct Test_t{
	glm::vec2 pos;
	glm::vec3 color;
}Test;

int main() {


	InitVolcano();

	ShaderGroup group = ShaderGroup(0);//will contain description of renderpass in the future, or nothing for default

	ID* id = new ID(0,0,PER_VERTEX,&Test,&Test.pos,&Test.color);
	Shader* shad = new Shader({id},&group,"src/shaders/mainShad");
	// Shader* secondShad = new Shader({id},&group,"Volcano/src/shaders/secondVertex.spv","Volcano/src/shaders/fragment.spv");


	VertexBuffer* buff = new VertexBuffer(id,3,Test);
	std::vector<glm::vec2> pos = {{0.0f, -0.5f},{0.5f, 0.5f},{-0.5f, 0.5f}};
	std::vector<glm::vec3> color = {{1.0f, 0.0f, 0.0f},{0.0f, 1.0f, 0.0f},{1.0f, 0.0f, 1.0f}};


	buff->WriteData(0,pos.data(),color.data());

	DrawObj* buffDraw = new DrawObj({buff},shad);
	DrawObj* drawSecond = new DrawObj({buff},shad);

	window = new Window("TestWindow");

	//game loop
	double afterTime = 0;
	double beforeTime = 0;
	double dt = 0;
	while (!glfwWindowShouldClose(window->window)) {
		//get delta time
		afterTime = glfwGetTime();
		dt = afterTime - beforeTime;
		beforeTime = afterTime;

		buffDraw->QueueDraw(window);
		drawSecond->QueueDraw(window);
		window->Draw();
		glfwPollEvents();
	}
	DestroyVolcano();
	return 0;
}
