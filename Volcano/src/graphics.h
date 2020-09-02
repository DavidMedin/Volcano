#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "errorCatch.h"
// #include "window.h"
#include "devices.h"
#include "list.h"
typedef struct SwapChain *SwapChain;
typedef struct Shader{
	VkPipelineLayout pipelineLayout;

	//index 0:vertex 1:frag 2:geom -- more in the future
	VkShaderModule* shadMods;
	unsigned int shadModCount;

	List* swapchains;//list of SwapChains
	List* graphicsPipelines;// VkPipline
	List* shaderDrawCommands;//list of an array of VkCommandBuffers
}*Shader;


typedef struct SwapChain{
	Device* device;
	VkSwapchainKHR swapChain;

	//indices into swapDets.formats and presentModes
	SwapChainSupportDetails* swapDets;
	VkExtent2D swapExtent;
	unsigned int chosenFormat;//use .format
	unsigned int chosenPresent;
	//images in the swapchain
	unsigned int imageCount;//count for both images and imageViews
	VkImage* images;//array of handles to VkImages
	VkImageView* imageViews;


	
	// VkFramebuffer* framebuffers;
	// unsigned int framebufferCount;//should equal to imageCount
	//links to shaders
	// unsigned int shaderCount;
	List* shaders;//list of Shaders
	unsigned int shaderCount;

/*
												count:imageCount*renderpassCount
					count:renderpassCount	 	 \------------\
	\------------\		  \--------\		-----\framebuffer \
	\framebuffers\------->\	ptr    \-------/	 \------------\
	\------------\		  \--------\			 \framebuffer \
			ptr			  \	ptr    \--\ 		 \------------\
						  \--------\   \---------\framebuffer \
												 \------------\
												 \framebuffer \
												 \------------\
*/										 
	unsigned int renderpassCount;
	VkRenderPass* renderpasses;
	//framebuffers[renderpassCount][imageCount]
	VkFramebuffer** framebuffers;//each renderpass needs a set of framebuffers
}*SwapChain;



int ReadTheFile(const char* path,char** buff,unsigned int* buffSize);
// void CreateRenderPass(Window win, Device* device,VkRenderPass* renderPass);

VkShaderModule CreateShaderModule(Device device,char* code,unsigned int codeSize);

void CreateShader(Device device,VkRenderPass renderpass, SwapChain swap,const char* vertexShader,const char* fragmentShader, Shader* shad);
// void CreateShader(Device device,VkRenderPass renderpass, SwapChain* swap, VkShaderModule vertexShader,VkShaderModule fragmentShader, Shader* shad);
void DestroyShader(Device device, Shader shad);

void CreateGraphicsPipeline(Device device,VkRenderPass renderPass,VkExtent2D viewExtent,Shader shad);

int CreateSwapChain(Device* device,VkSurfaceKHR surface, SwapChain* swapChain);
