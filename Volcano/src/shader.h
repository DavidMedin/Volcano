#pragma once
#include <vector>
#include <list>

#include "renderpass.h"
#include "globalVulkan.h"
#include "commandpool.h"
//#include "swapchain.h"
#define MAX_FRAMES_IN_FLIGHT 4

void CreateFramebuffers(VkDevice device, VkRenderPass render, VkImageView* imageViews, unsigned int imageCount, VkExtent2D extent, std::vector<VkFramebuffer>* framebuffIn);


struct VertexBuffer;
struct SwapChain;
struct Shader;
struct DrawTarget{
	Shader* shad;
	SwapChain* swapchain;
	std::shared_ptr<RenderPass> renderpass;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> frames;
	std::vector<VkCommandBuffer>* drawCommands;

	
	~DrawTarget();
};
struct Window;
struct Shader{
	Device* device;
	VkPipelineLayout pipelineLayout;

	//index 0:vertex 1:frag 2:geom -- more in the future
	VkShaderModule* shadMods;
	unsigned int shadModCount;
	ShaderGroup* group;

	VkCommandPool cmdPool;
	std::list<DrawTarget*> commands;

	unsigned int vertNum;
	std::list<VertexBuffer*> vertBuffs;

	Shader(ShaderGroup* shaderGroup,const char* vertexShader,const char* fragmentShader);
	~Shader();
	void RegisterVertexBuffer(VertexBuffer* buff);
	void RegisterSwapChain(SwapChain* swap);
	void DestroySwapChain(SwapChain* swap);
	void DrawFrame(Window* window);
};


int ReadTheFile(const char* path, char** buff, unsigned int* buffSize);
VkPipelineLayout CreatePipeLayout(Device* device);
VkPipeline CreateGraphicsPipeline(Device* device, VkPipelineLayout layout, VkRenderPass renderPass, VkExtent2D viewExtent, Shader* shad);

VkShaderModule CreateShaderModule(Device* device, char* code, unsigned int codeSize);
void DestroyShader(Device* device, Shader* shad);
void DestroyShaderModule();//need to be made