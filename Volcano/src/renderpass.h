#pragma once
#include "globalVulkan.h"
#include "window.h"
#include "framebuffer.h"
struct RenderPass{
    VkRenderPass renderpass;
    VkFormat format;
	unsigned int shaderGroup;
	RenderPass(Window* win,Device* device);
};
void RecreateRenderpass(SwapChain* swap,Framebuffer* framebuff);//format from swapchain and shader spec from framebuff to find whether there is a renderpass already made. otherwise make one with the 

// void CreateRenderPass(Window* win, Device* device,VkRenderPass* renderPass);