#include "source.h"
Window* window;

// struct Test_t{
// 	glm::vec2 pos;
// 	glm::vec3 color;
// }Test;

int main() {


	InitVolcano();

	ShaderGroup group = ShaderGroup(0);//will contain description of renderpass in the future, or nothing for default


	Shader* shad = new Shader(&group,"Volcano/src/shaders/mainShad");
	// Shader* secondShad = new Shader({id},&group,"Volcano/src/shaders/secondVertex.spv","Volcano/src/shaders/fragment.spv");


	VertexBuffer* posBuff = new VertexBuffer(new ID(0,0,1,BufferRate::PER_VERTEX,shad),3);
	VertexBuffer* colorAOffBuff = new VertexBuffer(new ID(1,2,2,BufferRate::PER_VERTEX,shad),3);
	
	VertexBuffer* posBuffOpp = new VertexBuffer(new ID(0,0,0,BufferRate::PER_VERTEX,shad),3);
	VertexBuffer* colorAOffBuffOpp = new VertexBuffer(new ID(1,1,2,BufferRate::PER_VERTEX,shad),3);
	
	std::vector<glm::vec2> pos = {{0.0f, -0.5f},{0.5f, 0.5f},{-0.5f, 0.5f}};
	std::vector<glm::vec2> offset = {{0.0f,1.0f},{.2f,0},{1.0f,.69f}};
	std::vector<glm::vec3> color = {{1.0f, 0.0f, 0.0f},{0.0f, 1.0f, 0.0f},{1.0f, 0.0f, 1.0f}};

	posBuff->WriteData(0,pos.data(),offset.data());
	colorAOffBuff->WriteData(0,color.data());

	posBuffOpp->WriteData(0,pos.data());
	colorAOffBuffOpp->WriteData(0,offset.data(),color.data());

	DrawObj* buffDraw = new DrawObj({posBuff,colorAOffBuff},shad);
	DrawObj* oppBuffs = new DrawObj({posBuffOpp,colorAOffBuffOpp},shad);

	window = new Window("TestWindow");
	Window* anotherOne = new Window("Anotha One!");

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
		oppBuffs->QueueDraw(window);
		buffDraw->QueueDraw(anotherOne);
		oppBuffs->QueueDraw(anotherOne);
		
		window->Draw();
		anotherOne->Draw();
		glfwPollEvents();
	}
	DestroyVolcano();
	return 0;
}
