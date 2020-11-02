#pragma once
#include <stdio.h>
#include <glm.hpp>
#include <vector>
#include <iostream>

#include "globalVulkan.h"
#include "errorCatch.h"

#define GETTYPE(T) typeid(T).hash_code()

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

template<class structType, class Last>
void Something(std::vector<VkVertexInputAttributeDescription>* vec, structType* inStruct, std::vector<void*>* list, unsigned int bufferLoc,unsigned int beginLoc,unsigned int index){
	//the last thing, same function as Something but no recurse
	unsigned int i = 0;
	for(auto member : *list){
		if(index == i){
			//does something using Last
			VkVertexInputAttributeDescription desc = {0};
			desc.binding = bufferLoc;
			desc.location = beginLoc+i;
			desc.format = GetTypeFormat<Last>();
			desc.offset = unsigned int((size_t)member-(size_t)inStruct);
			vec->push_back(desc);
		}
		i++;
	}
}

template <class structType, class First,class Second, class ...Rest>//second is to solve ambiguity
void Something(std::vector<VkVertexInputAttributeDescription>* vec, structType* inStruct, std::vector<void*>* list, unsigned int bufferLoc,unsigned int beginLoc,unsigned int index){
	unsigned int i = 0;
	for(auto member : *list){
		if(index == i){
			//does something using First
			VkVertexInputAttributeDescription desc = {0};
			desc.binding = bufferLoc;
			desc.location = beginLoc+i;
			desc.format = GetTypeFormat<First>();
			desc.offset = unsigned int((size_t)member-(size_t)inStruct);
			vec->push_back(desc);
			Something<structType,Second,Rest...>(vec,inStruct,list,bufferLoc,beginLoc,index+1);
		}
		i++;
	}
}


template <class structType, class ...TypesT>
void CreateVertexBuffer(structType* inStruct, std::initializer_list<void*> list, unsigned int bufferLoc,unsigned int beginLoc,BufferRate rate){
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

	std::vector<VkVertexInputAttributeDescription> inputDescs;
	std::vector<void*> listVec = std::vector<void*>(list);
	Something<structType,TypesT...>(&inputDescs,inStruct,&listVec,bufferLoc,beginLoc,0);

	
}