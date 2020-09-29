#pragma once
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <memory>

#include <vulkan/vulkan.h>

#include "errorCatch.h"
#include "commandpool.h"
// #include "window.h"
#include "devices.h"
#include "renderpass.h"
#define MAX_FRAMES_IN_FLIGHT 4

struct SwapChain;
struct Command{
	SwapChain* swapchain;
	std::shared_ptr<RenderPass> renderpass;
	VkPipeline graphicsPipeline;
	std::vector<VkCommandBuffer>* drawCommands;
	std::vector<VkFence> imageFence;

	std::vector<VkSemaphore> available;
	std::vector<VkSemaphore> presentable;
	std::vector<VkFence> fences;
};

struct Shader{
	Device* device;
	VkPipelineLayout pipelineLayout;

	//index 0:vertex 1:frag 2:geom -- more in the future
	VkShaderModule* shadMods;
	unsigned int shadModCount;
	unsigned int shaderGroup;

	VkCommandPool cmdPool;
	std::list<Command*> commands;

	Shader(Device* device,unsigned int shaderGroup, SwapChain* swap,const char* vertexShader,const char* fragmentShader);
	~Shader();
	void RegisterSwapChains(std::initializer_list<SwapChain*> swaps);
	void DrawFrame(SwapChain* window);
	void RemoveSwapChain();//need to be done
	void RecalculateSwapchain(SwapChain* swap);
};

struct Framebuffer{
	std::shared_ptr<RenderPass> renderpass;
	std::vector<VkFramebuffer> framebuffers;
};

struct SwapChain{
	Device* device;
	VkSwapchainKHR swapChain;
	VkSurfaceKHR surface;
	//indices into swapDets.formats and presentModes
	SwapChainSupportDetails swapDets;
	VkExtent2D swapExtent;
	unsigned int chosenFormat;
	unsigned int chosenPresent;

	//images in the swapchain
	unsigned int imageCount;//count for both images and imageViews
	VkImage* images;
	VkImageView* imageViews;

	std::list<Shader*> shaders;//shaders to recreate on recreation
	std::list<Framebuffer*> frames;
	SwapChain(Device* device,VkSurfaceKHR surface);
	~SwapChain();
	void RegisterRenderPasses(std::initializer_list<std::shared_ptr<RenderPass>> renderpasses);
	void Recreate();
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
