#pragma once
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>

#include <vulkan/vulkan.h>

#include "errorCatch.h"
#include "commandpool.h"
// #include "window.h"
#include "devices.h"
// typedef struct{} *SwapChain;
struct SwapChain;
struct Shader{
	Device* device;
	VkRenderPass renderpass;
	VkPipelineLayout pipelineLayout;

	//index 0:vertex 1:frag 2:geom -- more in the future
	VkShaderModule* shadMods;
	unsigned int shadModCount;

	std::list<SwapChain*> swapchains;
	std::list<VkPipeline> graphicsPipelines;// VkPipline
	VkCommandPool cmdPool;
	std::list<std::vector<VkCommandBuffer>*> drawCommands;//list of an array of VkCommandBuffers

	Shader(Device* device,VkRenderPass renderpass, SwapChain* swap,const char* vertexShader,const char* fragmentShader);
	void RegisterSwapChains(std::initializer_list<SwapChain*> swaps);
	void RemoveSwapChain();//need to be done
	void RecalculateSwapChain();//need to be done
};


struct SwapChain{
	Device* device;
	VkSwapchainKHR swapChain;
	//indices into swapDets.formats and presentModes
	SwapChainSupportDetails* swapDets;
	VkExtent2D swapExtent;
	unsigned int chosenFormat;
	unsigned int chosenPresent;

	//images in the swapchain
	unsigned int imageCount;//count for both images and imageViews
	VkImage* images;
	VkImageView* imageViews;

	std::list<Shader*> shaders;//shaders to recreate on recreation

	std::list<VkRenderPass> renderpasses;
	std::list<std::vector<VkFramebuffer>*> framebuffers;

	SwapChain(Device* device,VkSurfaceKHR surface);
	void RegisterRenderPasses(std::initializer_list<VkRenderPass> renderpasses);
	void RecalcuateRenderPasses();
};



int ReadTheFile(const char* path,char** buff,unsigned int* buffSize);
// void CreateRenderPass(Window win, Device* device,VkRenderPass* renderPass);


// void CreateShader(Device device,VkRenderPass renderpass, SwapChain* swap, VkShaderModule vertexShader,VkShaderModule fragmentShader, Shader* shad);
VkPipelineLayout CreatePipeLayout(Device* device);
VkPipeline CreateGraphicsPipeline(Device* device,VkPipelineLayout layout, VkRenderPass renderPass,VkExtent2D viewExtent,Shader* shad);

VkShaderModule CreateShaderModule(Device* device,char* code,unsigned int codeSize);
void DestroyShader(Device* device, Shader* shad);
void DestroyShaderModule();//need to be made
// void CreateShader(Device device,VkRenderPass renderpass, SwapChain swap,const char* vertexShader,const char* fragmentShader, Shader* shad);
// int CreateSwapChain(Device* device,VkSurfaceKHR surface, SwapChain* swapChain);
