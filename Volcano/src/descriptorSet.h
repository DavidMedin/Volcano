#pragma once
#include <stdio.h>
#include <glm.hpp>
#include <vector>

#include "globalVulkan.h"
#include "errorCatch.h"
struct Device;

//should probably have a max poolSize as well
VkDescriptorPool CreateDescriptorPool(Device* device,int poolSize);


struct UniformBuffer{//this is a binding
	VkBuffer buff;
	//stages look like VK_SHADER_STAGE_VERTEX_BIT, where VERTEX can be FRAGMENT or whatever stages (you can | them together)
	UniformBuffer(VkDescriptorPool pool,unsigned int binding,VkShaderStageFlags stages);
};

//function in shader to get descriptorset with set #
struct DescriptorSet{
	VkDescriptorSetLayout layout;
	std::vector<UniformBuffer> uniforms;
};