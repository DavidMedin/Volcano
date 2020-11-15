#include "vertexbuffer.h"
#include "shader.h"


VertexBuffer::~VertexBuffer(){
	for(auto shad : shaders){
		//should remove all refs from shaders
		shad->vertBuffs.remove(this);
	}
	vkDestroyBuffer(device->device,buff,NULL);
	vkFreeMemory(device->device,buffMem,NULL);
}

void VertexBuffer::MapData(void** data){
	vkMapMemory(device->device,buffMem,0,memSize,0,data);
}
void VertexBuffer::UnMapData(){
	vkUnmapMemory(device->device,buffMem);
}
