#include "descriptorSet.h"
#include "devices.h"

VkDescriptorPool CreateDescriptorPool(Device* device,int poolSize){
	VkDescriptorPoolSize poolSizeInfo{};
	poolSizeInfo.descriptorCount = poolSize;
	poolSizeInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSizeInfo;
	poolInfo.maxSets = poolSize;//weirdo
	VkDescriptorPool pool;
	if(vkCreateDescriptorPool(device->device,&poolInfo,NULL,&pool) != VK_SUCCESS){
		Error("Couldn't create Descriptor Pool\n");
		return nullptr;
	}
	return pool;
}
UniformBuffer::UniformBuffer(VkDescriptorPool pool,unsigned int binding,VkShaderStageFlags stages){

}
