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

	DescriptorSet* set = new DescriptorSet(shad,0);

	IndexBuffer* index = new IndexBuffer(6);
	VertexBuffer* buff = new VertexBuffer(new ID(0,0,1,BufferRate::PER_VERTEX,shad),4);

	std::vector<glm::vec2> pos = {{-0.5f, -0.5f},{0.5f, -0.5f},{0.5f, 0.5f},{-0.5f,0.5f}};
	std::vector<glm::vec3> color = {{1.0f, 0.0f, 0.0f},{0.0f, 1.0f, 0.0f},{1.0f, 0.0f, 1.0f},{.835f,.687f,.6914f}};

	std::vector<uint32_t> indices = {0,1,2,0,2,3};
	void* data;
	index->MapData(&data);
	memcpy(data,indices.data(),indices.size()*sizeof(uint32_t));
	index->UnMapData();

	buff->WriteData(0,pos.data(),color.data());

	DrawObj* buffDraw = new DrawObj({buff},index,shad);

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
		buffDraw->QueueDraw(anotherOne);

		window->Draw();
		anotherOne->Draw();
		glfwPollEvents();
	}
	DestroyVolcano();
	return 0;
}
