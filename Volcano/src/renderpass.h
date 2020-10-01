#pragma once
#include "globalVulkan.h"
// #include "window.h"
// #include "framebuffer.h"
#include "Devices.h"
// extern struct SwapChain;
struct RenderPass{
	VkDevice device;

    VkRenderPass renderpass;
    VkFormat format;
	unsigned int shaderGroup;
	// RenderPass(Window* win,Device* device);
};
void DestroyRenderpasses();//this is a terrible function.
std::shared_ptr<RenderPass> GetRenderpass(VkFormat format,Device* device,unsigned int shaderGroup);//format from swapchain and shader spec from framebuff to find whether there is a renderpass already made. otherwise make one with the 

// void CreateRenderPass(Window* win, Device* device,VkRenderPass* renderPass);