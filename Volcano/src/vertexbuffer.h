#pragma once
#include <stdio.h>
#include <glm.hpp>
#include <vector>
#include <iostream>
#include <list>
#include <initializer_list>

#include "devices.h"
#include "globalVulkan.h"
#include "errorCatch.h"
#include "commandpool.h"

#define GETTYPE(T) typeid(T).hash_code()

struct Shader;

enum BufferRate{
	PER_VERTEX,
	PER_INSTANCE
};

template <class T>
VkFormat GetTypeFormat(){
	size_t type = GETTYPE(T);
	if(type == GETTYPE(float)){
		return VK_FORMAT_R32_SFLOAT;
	}else if(type == GETTYPE(int)){
		return VK_FORMAT_R32_SINT;
	}else if(type == GETTYPE(char)){
		return VK_FORMAT_R32_SINT;
	}else if(type == GETTYPE(glm::vec2)){
		return VK_FORMAT_R32G32_SFLOAT;
	}else if(type == GETTYPE(glm::vec3)){
		return VK_FORMAT_R32G32B32_SFLOAT;
	}else if(type == GETTYPE(glm::vec4)){
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}else{
		Error("oh no! \"%s\" is not a valid argument for GetTypeFormat!\n",typeid(T).name());
		return VK_FORMAT_UNDEFINED;
	}
}

void CreateBuffer(Device* device,uint64_t size, int usage,VkSharingMode share,VkMemoryPropertyFlags props,VkBuffer* buff,VkDeviceMemory* buffMem);

//InputDescription
struct ID{
	VkVertexInputBindingDescription bindDesc;
	std::vector<VkVertexInputAttributeDescription> attribDescs;
	size_t size;
	//bad stupid code. You should be using shaders to infer your input descriptions, but this is here if you want to define your own for some reason
	template<class structType, class Last>
	int ItterAtrib(std::vector<VkVertexInputAttributeDescription>* vec,  unsigned int bufferLoc,unsigned int 	beginLoc,unsigned int index,structType* inStruct,Last* last){
		//the last thing, same function as Something but no recurse
		VkVertexInputAttributeDescription desc = {0};
		desc.binding = bufferLoc;
		desc.location = beginLoc+index;
		desc.format = GetTypeFormat<Last>();
		desc.offset = unsigned int((size_t)last-(size_t)inStruct);
		vec->push_back(desc);
		return desc.offset+sizeof(Last);
	}

	template <class structType, class First, class ...Rest>//second is to solve ambiguity
	int ItterAtrib(std::vector<VkVertexInputAttributeDescription>* vec,  unsigned int bufferLoc,unsigned int beginLoc,unsigned int index,structType* inStruct,First* first, Rest*... rest){
		VkVertexInputAttributeDescription desc = {0};
		desc.binding = bufferLoc;
		desc.location = beginLoc+index;
		desc.format = GetTypeFormat<First>();
		desc.offset = unsigned int((size_t)first-(size_t)inStruct);
		vec->push_back(desc);
		return ItterAtrib<structType,Rest...>(vec,bufferLoc,beginLoc,index+1,inStruct,rest...);
	}
	
	template <class structType, class ...TypesT>
	ID(unsigned int bufferLoc,unsigned int beginLoc,BufferRate rate,structType* inStruct, TypesT*... data){
		bindDesc = {0};
		bindDesc.binding = bufferLoc;
		bindDesc.stride = sizeof(structType);
		size = bindDesc.stride;
		switch(rate){
			case PER_VERTEX:
				bindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				break;
			case PER_INSTANCE:
				bindDesc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
				break;
			default:
				Error("That BufferRate is not supported!\n");
		}

		ItterAtrib<structType,TypesT...>(&attribDescs,bufferLoc,beginLoc,0,inStruct,data...);

	}
	ID(unsigned int bufferLoc,unsigned int beginLoc,unsigned int endLoc,BufferRate rate,Shader* shad);
};



struct Buffer{
	Device* device;
	VkBuffer stageBuff;
	VkDeviceMemory stageMem;
	VkBuffer fastBuff;
	VkDeviceMemory fastMem;
	uint64_t memSize;
	unsigned int vertexNum;
	Buffer(Device* device,unsigned int vertexNum,uint64_t size, int usage);
	~Buffer();

	void MapData(void** data);
	void UnMapData();
};
struct IndexBuffer{
	Device* device;
	
	Buffer* indexBuff;
	uint64_t memSize;
	unsigned int indexCount;
	void MapData(void** data);
	void UnMapData();
	IndexBuffer(unsigned int indexCount){
		this->indexCount = indexCount;
		memSize = indexCount*sizeof(uint32_t);
		device = GetCurrentDevice();
		indexBuff = new Buffer(device,indexCount,memSize,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	}
	
};

struct VertexBuffer {
	Device* device;
	Buffer* buff;
	uint64_t memSize;
	unsigned int vertexNum;	//default is num of first
	ID* inDesc;

	template<class last>
	void IterThruArgs(void* allocData, int index, unsigned int vertDex, last* lastArg) {
		memcpy((void*)(size_t(allocData) + inDesc->attribDescs[index].offset + (inDesc->bindDesc.stride * vertDex)), (void*)(size_t(lastArg) + sizeof(last) * vertDex), sizeof(last));
	}
	template< class next, class ...argsT>
	void IterThruArgs(void* allocData, int index, unsigned int vertDex, next* nextArg, argsT*... data) {
		memcpy((void*)(size_t(allocData) + inDesc->attribDescs[index].offset + (inDesc->bindDesc.stride * vertDex)), (void*)(size_t(nextArg) + sizeof(next) * vertDex), sizeof(next));
		IterThruArgs(allocData, index + 1, vertDex, data...);
	}
	template<class ...argsT>
	void* FormatVertexBuffer(unsigned int mask, argsT*... data) {
		//(x[],y[],z[]) -> data
		//(x[0],y[0],z[0]),
		//(x[1],y[1],z[1])
		void* allocData = malloc(memSize);
		for (unsigned int i = 0; i < vertexNum; i++) {
			IterThruArgs(allocData, 0, i, data...);
		}
		return allocData;
	}

	template<class ...argsT>
	void WriteData(unsigned int mask, argsT*... data) {
		//mask is untested
		void* formatData = FormatVertexBuffer(mask, data...);
		void* mapped;
		buff->MapData(&mapped);
		memcpy(mapped, formatData, memSize);
		buff->UnMapData();
		free(formatData);
	}

	VertexBuffer(ID* id,unsigned int vertNum){
		inDesc = id;
		device = GetCurrentDevice();
		memSize = id->size*vertNum;
		vertexNum = vertNum;
		buff = new Buffer(device,vertexNum,memSize,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	}


	// ~VertexBuffer();
};

