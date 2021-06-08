#pragma once
#include <stdio.h>
#include <glm.hpp>
#include <vector>
#include <list>

#include "globalVulkan.h"
#include "errorCatch.h"
#include "spirv_reflect.h"
struct Device;
struct Shader;
struct SwapChain;
struct DescriptorSet;

//should probably have a max poolSize as well

struct DescriptorBinding{
	DescriptorSet* set;
	SpvReflectDescriptorBinding* bindingInfo;
};

VkDescriptorPool CreateDescriptorPool(Device* device,int imageCount,std::vector<DescriptorBinding> descTypes);

struct DescriptorSet{
	Device* device;
	VkDescriptorSetLayout layout;

	struct SetPerSwap{
		SwapChain* swap;
		VkDescriptorPool pool;
		struct SetBuffers{
			VkBuffer buff;
			VkDeviceMemory mem;
		};
		std::vector<std::vector<SetBuffers>> mems;//[imageCount[bindingCount]]
		// std::vector<VkBuffer> buffs;
		// std::vector<VkDeviceMemory> mems;
		std::vector<VkDescriptorSet> sets;

	};
	std::list<SetPerSwap> swaps;
	std::vector<DescriptorBinding> bindings;
	
	DescriptorSet(Shader* shad,int setNum);
	void RegisterSwap(SwapChain* swap);
	void RemoveSwapChain(SwapChain* swap);
};

//don't forget to recreate with the swapchain!