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
void CreateVertexBuffer(Shader* shad, unsigned int bufferLoc,unsigned int beginLoc,BufferRate rate,structType* inStruct, TypesT*... data){
	VkVertexInputBindingDescription bindingDesc = {0};
	bindingDesc.binding = bufferLoc;
	bindingDesc.stride = sizeof(inStruct);
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
	buff->bindDesc = bindingDesc;
	int size = Something<structType,TypesT...>(&buff->attribDescs,bufferLoc,beginLoc,0,inStruct,data...);
	shad->RegisterVertexBuffer(buff);
	//create the buffer
	VkBufferCreateInfo buffInfo = {0};
	buffInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffInfo.size = size;
	buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//cause only for graphics queue
	if(vkCreateBuffer(shad->device->device,&buffInfo,NULL,&buff->buff) != VK_SUCCESS){
		Error("Couldn't create a vertex buffer!\n");
	}
}
