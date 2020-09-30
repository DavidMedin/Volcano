#pragma once
#include "globalVulkan.h"
// #include "window.h"
// #include "framebuffer.h"
extern struct SwapChain;
struct RenderPass{
	VkDevice device;

    VkRenderPass renderpass;
    VkFormat format;
	unsigned int shaderGroup;
	// RenderPass(Window* win,Device* device);
};
std::shared_ptr<RenderPass> GetRenderpass(SwapChain* swap,Device* device,unsigned int shaderGroup);//format from swapchain and shader spec from framebuff to find whether there is a renderpass already made. otherwise make one with the 

// void CreateRenderPass(Window* win, Device* device,VkRenderPass* renderPass);