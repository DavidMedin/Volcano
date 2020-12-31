#include "source.h"
Window* window;

struct Test_t{
	glm::vec2 pos;
	glm::vec3 color;
}Test;

int main() {


	InitVolcano();

	ShaderGroup group = ShaderGroup(0);//will contain description of renderpass in the future, or nothing for default

	ID* posID = new ID(0,0,PER_VERTEX,&Test,&Test.pos);
	ID* colorID = new ID(1,1,PER_VERTEX,&Test,&Test.color);

	Shader* shad = new Shader({posID,colorID},&group,"Volcano/src/shaders/mainShad");
	// Shader* secondShad = new Shader({id},&group,"Volcano/src/shaders/secondVertex.spv","Volcano/src/shaders/fragment.spv");


	VertexBuffer* posBuff = new VertexBuffer(shad->GetNthID(0),3);
	VertexBuffer* colorBuff = new VertexBuffer(shad->GetNthID(1),3);
	std::vector<glm::vec2> pos = {{0.0f, -0.5f},{0.5f, 0.5f},{-0.5f, 0.5f}};
	std::vector<glm::vec3> color = {{1.0f, 0.0f, 0.0f},{0.0f, 1.0f, 0.0f},{1.0f, 0.0f, 1.0f}};


	posBuff->WriteData(0,pos.data());
	colorBuff->WriteData(0,color.data());

	DrawObj* buffDraw = new DrawObj({posBuff,colorBuff},shad);

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
		window->Draw();
		glfwPollEvents();
	}
	DestroyVolcano();
	return 0;
}
