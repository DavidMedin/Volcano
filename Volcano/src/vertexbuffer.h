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

struct Shader;
struct VertexBuffer {

	Device* device;

	VkBuffer stageBuff;
	VkDeviceMemory stageMem;


	VkBuffer fastBuff;
	VkDeviceMemory fastBuffMem;
	//requires vertexdata to make this
	uint64_t memSize;
	void* formattedData;

	//default is num of first
	unsigned int vertexNum;

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

	//to write use:
	void MapData(void** data);//then
	void UnMapData();
	//OR:
	template<class ...argsT>
	void WriteData(unsigned int mask, argsT*... data) {
		void* formatData = FormatVertexBuffer(mask, data...);
		void* mapped;
		MapData(&mapped);
		memcpy(mapped, formatData, memSize);
		UnMapData();
		free(formatData);
	}


	//don't touch
	void AddToList();

	// template <class structType>
	VertexBuffer(ID* id,unsigned int vertNum){
	inDesc = id;
	device = GetCurrentDevice();
	// memSize = sizeof(structType)*vertNum;
	memSize = id->size*vertNum;
	vertexNum = vertNum;
	//create the staging buff buffer
	CreateBuffer(device,memSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VK_SHARING_MODE_EXCLUSIVE,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,&stageBuff,&stageMem);

	CreateBuffer(device,memSize,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VK_SHARING_MODE_EXCLUSIVE,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,&fastBuff,&fastBuffMem);

	AddToList();
	}


	~VertexBuffer();
};

