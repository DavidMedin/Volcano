#pragma once
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <memory>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "errorCatch.h"
#include "commandpool.h"
// #include "window.h"
#include "devices.h"
#include "renderpass.h"
#define MAX_FRAMES_IN_FLIGHT 4

struct SwapChain;
struct Draw{
	SwapChain* swapchain;
	std::shared_ptr<RenderPass> renderpass;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> frames;
	std::vector<VkCommandBuffer>* drawCommands;

	std::vector<VkFence> imageFence;

	std::vector<VkSemaphore> available;
	std::vector<VkSemaphore> presentable;
	std::vector<VkFence> fences;
	~Draw();
};

struct Shader{
	Device* device;
	VkPipelineLayout pipelineLayout;

	//index 0:vertex 1:frag 2:geom -- more in the future
	VkShaderModule* shadMods;
	unsigned int shadModCount;
	ShaderGroup* group;

	VkCommandPool cmdPool;
	std::list<Draw*> commands;

	Shader(Device* device,ShaderGroup* shaderGroup,const char* vertexShader,const char* fragmentShader);
	~Shader();
	void RegisterSwapChain(SwapChain* swap);
	void DestroySwapChain(SwapChain* swap);
	void DrawFrame(SwapChain* window);
};

struct SwapChain{
	Device* device;
	VkSwapchainKHR swapChain;
	VkSurfaceKHR surface;
	GLFWwindow* win;
	//indices into swapDets.formats and presentModes
	SwapChainSupportDetails swapDets;
	VkExtent2D swapExtent;
	bool windowResized;
	unsigned int chosenFormat;
	unsigned int chosenPresent;

	//images in the swapchain
	unsigned int imageCount;//count for both images and imageViews
	VkImage* images;
	VkImageView* imageViews;

	SwapChain(Device* device,VkSurfaceKHR surface);
	~SwapChain();
	void Recreate();
	VkFormat GetFormat();
};



int ReadTheFile(const char* path,char** buff,unsigned int* buffSize);
VkPipelineLayout CreatePipeLayout(Device* device);
VkPipeline CreateGraphicsPipeline(Device* device,VkPipelineLayout layout, VkRenderPass renderPass,VkExtent2D viewExtent,Shader* shad);

VkShaderModule CreateShaderModule(Device* device,char* code,unsigned int codeSize);
void DestroyShader(Device* device, Shader* shad);
void DestroyShaderModule();//need to be made
