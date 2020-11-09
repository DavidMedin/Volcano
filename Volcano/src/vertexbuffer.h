#pragma once
#include <stdio.h>
#include <glm.hpp>
#include <vector>
#include <iostream>

#include "globalVulkan.h"
#include "errorCatch.h"
#include "graphics.h"

#define GETTYPE(T) typeid(T).hash_code()

enum BufferRate{
	PER_VERTEX,
	PER_INSTANCE
};

// struct VertexBuffer{
// 	VkVertexInputBindingDescription bindDesc;
// 	std::vector<VkVertexInputAttributeDescription> attribDescs;
// };

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

template<class structType, class Last>
int Something(std::vector<VkVertexInputAttributeDescription>* vec,  unsigned int bufferLoc,unsigned int beginLoc,unsigned int index,structType* inStruct,Last* last){
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
int Something(std::vector<VkVertexInputAttributeDescription>* vec,  unsigned int bufferLoc,unsigned int beginLoc,unsigned int index,structType* inStruct,First* first, Rest*... rest){
	VkVertexInputAttributeDescription desc = {0};
	desc.binding = bufferLoc;
	desc.location = beginLoc+index;
	desc.format = GetTypeFormat<First>();
	desc.offset = unsigned int((size_t)first-(size_t)inStruct);
	vec->push_back(desc);
	return Something<structType,Rest...>(vec,bufferLoc,beginLoc,index+1,inStruct,rest...);
}


template <class structType, class ...TypesT>
VertexBuffer* CreateVertexBuffer(Shader* shad,unsigned int vertNum, unsigned int bufferLoc,unsigned int beginLoc,BufferRate rate,structType* inStruct, TypesT*... data){
	VkVertexInputBindingDescription bindingDesc = {0};
	bindingDesc.binding = bufferLoc;
	bindingDesc.stride = sizeof(structType);
	switch(rate){
		case PER_VERTEX:
			bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			break;
		case PER_INSTANCE:
			bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
			break;
		default:
			Error("That BufferRate is not supported!\n");
	}

	// std::vector<VkVertexInputAttributeDescription> inputDescs;
	VertexBuffer* buff = new VertexBuffer;
	buff->uses = 0;
	buff->vertexNum = vertNum;
	buff->device = shad->device;
	buff->bindDesc = bindingDesc;
	Something<structType,TypesT...>(&buff->attribDescs,bufferLoc,beginLoc,0,inStruct,data...);
	shad->RegisterVertexBuffer(buff);
	//create the buffer
	VkBufferCreateInfo buffInfo = {0};
	buffInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffInfo.size = sizeof(structType);
	buff->memSize = sizeof(structType)*vertNum;
	buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//cause only for graphics queue
	if(vkCreateBuffer(shad->device->device,&buffInfo,NULL,&buff->buff) != VK_SUCCESS){
		Error("Couldn't create a vertex buffer!\n");
	}
	//mem requirements
	VkMemoryRequirements memRequire;
	vkGetBufferMemoryRequirements(shad->device->device,buff->buff,&memRequire);

	//find a place in the GPU that matches our vertex buffer memory requirements
	unsigned int properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	int typeIndex = -1;
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(shad->device->phyDev,&memProps);
	for(int i = 0; i < (int)memProps.memoryTypeCount;i++){
		if(memRequire.memoryTypeBits & (1 << i) && (memProps.memoryTypes[i].propertyFlags & properties)==properties){
			typeIndex = i;
			break;
		}
	}
	if(typeIndex == -1) {Error("No memory types work for this vertex buffer. Or you're just stupid\n");}

	//allocate memory on the GPU for the buffer
	VkMemoryAllocateInfo allocInfo = {0};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequire.size;
	allocInfo.memoryTypeIndex = typeIndex;

	if(vkAllocateMemory(shad->device->device,&allocInfo,NULL,&buff->buffMem) != VK_SUCCESS){
		Error("Couldn't allocate memory for the vertex buffer!\n");
	}	

	vkBindBufferMemory(device->device,buff->buff,buff->buffMem,0);

	return buff;
}
