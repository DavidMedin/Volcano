#pragma once
#include <stdio.h>
#include <glm.hpp>
#include <tuple>
#include <iostream>

#include "globalVulkan.h"
#include "errorCatch.h"

enum BufferRate{
	PER_VERTEX,
	PER_INSTANCE
};

template <class ...types>
struct MultiType{


	template<class ...types>
	MultiType(types&... args...){
		
	}
};


template <class T>
VkFormat GetTypeFormat(){
	switch(T){
		case float:
			return VK_FORMAT_R32_SFLOAT;
		case glm::vec2:
			return VK_FORMAT_R32G32_SFLOAT;
		case glm::vec3:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case glm::vec4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		default:
			Error("That type is not supported!\n");
			return VK_NULL;
	}
}

template <class structType, class ...TypesT>
void CreateVertexBuffer(structType* inStruct, std::tuple<TypesT*...> list, unsigned int bufferLoc,unsigned int beginLoc,BufferRate rate){
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
	unsigned int i = 0;
	while(std::get<i>(list) != NULL){
		auto member = std::get<i>(list);
		VkVertexInputAttributeDescription desc = {0};
		desc.binding = bindingDesc;
		desc.location = beginLoc+i;
		desc.format = GetTypeFormat<typeid(member)>();
		desc.offset = member-inStruct;
		i++;
	}
}