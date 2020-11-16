#pragma once
#include <vector>
#include <list>

#include "globalVulkan.h"
#include "Devices.h"
struct ShaderGroup{
	//will contain info on the renderpass later
	int index; //not actually used. will replace with renderpass info later
};

struct RenderPass{
	VkDevice device;

    VkRenderPass renderpass;
    VkFormat format;
	// unsigned int shaderGroup;
	ShaderGroup* group;
	// RenderPass(Window* win,Device* device);
};
void DestroyRenderpasses();//this is a terrible function.
std::shared_ptr<RenderPass> GetRenderpass(VkFormat format,Device* device,ShaderGroup* group);//format from swapchain and shader spec from framebuff to find whether there is a renderpass already made. otherwise make one with the 
std::shared_ptr<RenderPass> CreateRenderpass(VkFormat format,Device* device,ShaderGroup* group);