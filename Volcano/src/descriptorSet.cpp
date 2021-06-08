#include "descriptorSet.h"
#include "devices.h"
#include "shader.h"
#include "swapchain.h"
#include "vertexbuffer.h"
VkDescriptorPool CreateDescriptorPool(Device* device,int imageCount,std::vector<DescriptorBinding> descTypes){
	// VkDescriptorPoolSize poolSizeInfo{};
	// poolSizeInfo.descriptorCount = imageCount;
	// poolSizeInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	std::vector<VkDescriptorPoolSize> poolSizes(descTypes.size());
	for(auto desc : descTypes){
		VkDescriptorPoolSize poolSizeInfo{};
		poolSizeInfo.descriptorCount = imageCount;
		poolSizeInfo.type = (VkDescriptorType)desc.bindingInfo->descriptor_type;
		poolSizes.push_back(poolSizeInfo);
	}
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = imageCount;//weirdo
	VkDescriptorPool pool;
	if(vkCreateDescriptorPool(device->device,&poolInfo,NULL,&pool) != VK_SUCCESS){
		Error("Couldn't create Descriptor Pool\n");
		return nullptr;
	}
	return pool;
}

DescriptorSet::DescriptorSet(Shader* shad,int setNum){
	std::vector<VkDescriptorType> types;
	int q = 0;
	for(auto thing : shad->descriptorSetDescs){
		if(thing->set == setNum){
			for(int i = 0;i < thing->binding_count;i++){
				SpvReflectDescriptorBinding* binding = thing->bindings[i];
				types.push_back((VkDescriptorType)binding->descriptor_type);

				DescriptorBinding tmp;
				tmp.set = this;
				tmp.bindingInfo = binding;
				bindings.push_back(tmp);
				// printf("%s member[0]: %s\n",binding->block.name,binding->block.members[0].name);
			}
			layout = shad->descriptorSetLayouts[q];
		}
		q++;		
	}
	device = shad->device;
	shad->createdSets.push_back(this);//so a swapchain is able to find it, and only once
}
void DescriptorSet::RegisterSwap(SwapChain* swap){
	//check if swap already is in, recalc if so
	for(auto perSwap : swaps){
		if(perSwap.swap == swap){
			printf("Recreating Descriptor -> destroying then creating\n");
			RemoveSwapChain(swap);
		}
	}
	SetPerSwap* tmpSwap = new SetPerSwap;
	tmpSwap->swap = swap;
	tmpSwap->pool = CreateDescriptorPool(device,swap->imageCount,bindings);
	tmpSwap->mems.resize(swap->imageCount);
	for(int i = 0;i < swap->imageCount;i++){
		tmpSwap->mems[i].resize(bindings.size());
		for(int bind = 0;bind < bindings.size();bind++){
			int usage=0;
			switch(bindings[bind].bindingInfo->descriptor_type){
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
					break;
				default: 
					Error("That (%d) descriptor type is not supported!\n",bindings[bind].bindingInfo->descriptor_type);
			}
			CreateBuffer(device,bindings[bind].bindingInfo->block.size,usage,VK_SHARING_MODE_EXCLUSIVE,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,&tmpSwap->mems[i][bind].buff,&tmpSwap->mems[i][bind].mem);
		}
		
	}
	//allocate memory
	std::vector<VkDescriptorSetLayout> copyLayout(swap->imageCount,layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = tmpSwap->pool;
	allocInfo.descriptorSetCount = swap->imageCount;
	allocInfo.pSetLayouts = copyLayout.data();

	tmpSwap->sets.resize(swap->imageCount);
	if(vkAllocateDescriptorSets(device->device,&allocInfo,tmpSwap->sets.data())!=VK_SUCCESS){
		Error("Couldn't allocate descriptor set memory!\n");
	}
	//bind descriptor sets to buffers/memory
	std::vector<VkDescriptorBufferInfo> bufferInfos(swap->imageCount*bindings.size());
	std::vector<VkWriteDescriptorSet> descWrites(swap->imageCount*bindings.size());
	int setNum=0;
	for(auto image : tmpSwap->mems){
		int bind=0;
		for(auto binding : image){
			VkDescriptorBufferInfo* bufferInfo = &bufferInfos[setNum*bindings.size()+bind];
			bufferInfo->buffer = binding.buff;
			bufferInfo->offset = 0;//I will return, someday
			bufferInfo->range = bindings[bind].bindingInfo->block.size;
			VkWriteDescriptorSet* descWrite = &descWrites[setNum*bindings.size()+bind];
			descWrite->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descWrite->dstSet = tmpSwap->sets[setNum];
			descWrite->dstBinding = bind;
			descWrite->dstArrayElement = 0;//will add sometime later I guess
			descWrite->descriptorType=(VkDescriptorType)bindings[bind].bindingInfo->descriptor_type;
			descWrite->descriptorCount=1;//num of elements to write in in dstArrayElement
			descWrite->pBufferInfo = bufferInfo;
			bind++;
		}
		setNum++;
	}
	vkUpdateDescriptorSets(device->device,descWrites.size(),descWrites.data(),0,nullptr);//dunno what copies are
}
void DescriptorSet::RemoveSwapChain(SwapChain* swap){
	//free buffers and memory
	for(auto checkSwap : swaps){
		if(checkSwap.swap == swap){
			//found our swapchain in the descriptorSet
			for(auto img : checkSwap.mems){
				for(auto binding : img){
					vkDestroyBuffer(device->device,binding.buff,nullptr);
					vkFreeMemory(device->device,binding.mem,nullptr);
				}
			}
			//destroy pool
			vkDestroyDescriptorPool(device->device,checkSwap.pool,nullptr);
			return;
		}
	}
	Error("Couldn't find swapchain registered in descriptor set");
}