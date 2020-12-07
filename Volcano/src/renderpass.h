#pragma once
#include <vector>
#include <list>

#include "globalVulkan.h"
#include "Devices.h"

struct ShaderGroup{
	//will contain info on the renderpass later
	VkRenderPassCreateInfo* defaultpass;
	VkRenderPassCreateInfo* clearerpass;

	int index; //not actually used. will replace with renderpass info later
	ShaderGroup();
	ShaderGroup(int index);
	ShaderGroup(int index,VkRenderPassCreateInfo* default,VkAttachmentDescription* presentAttach);
	ShaderGroup(int index,VkRenderPassCreateInfo* default,VkRenderPassCreateInfo* clearer);
};

struct RenderPass{
	VkDevice device;

    VkRenderPass renderpass;
	VkRenderPass clearpass;
    VkFormat format;
	// unsigned int shaderGroup;
	ShaderGroup* group;
	RenderPass(ShaderGroup* group);
	// RenderPass(Window* win,Device* device);
};
void DestroyRenderpasses();//this is a terrible function.
std::shared_ptr<RenderPass> GetRenderpass(VkFormat format,Device* device,ShaderGroup* group);//format from swapchain and shader spec from framebuff to find whether there is a renderpass already made. otherwise make one with the 
std::shared_ptr<RenderPass> CreateRenderpass(VkFormat format,Device* device,ShaderGroup* group);