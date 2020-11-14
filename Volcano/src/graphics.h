#pragma once
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <memory>
#include <tuple>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "errorCatch.h"
// #include "commandpool.h"
// #include "window.h"
#include "devices.h"
#include "renderpass.h"
// #include "vertexbuffer.h"
#define MAX_FRAMES_IN_FLIGHT 4

struct Shader;


VkCommandPool CreateCommandPool(VkDevice device,QueueFamilyIndex* indices);

std::vector<VkCommandBuffer>* CreateCommandBuffers(Device* device,VkCommandPool commandPool,unsigned int commandBuffCount);

void FillCommandBuffers(VkExtent2D swapChainExtent,std::vector<VkFramebuffer>* frameBuffs,VkPipeline graphicsPipeline,VkRenderPass renderPass,Shader* shad,std::vector<VkCommandBuffer>* cmdBuffs);

int CreateSemaphore(VkDevice device, VkSemaphore* semaphore);
int CreateFence(VkDevice device,VkFence* fence);

struct VertexBuffer{
	std::list<Shader*> shaders;
	unsigned int uses;

	Device* device;
	VkBuffer buff;
	VkDeviceMemory buffMem;
	uint64_t memSize;

	//requires vertexdata to make this
	void* formattedData;

	//default is num of first
	unsigned int vertexNum;

	VkVertexInputBindingDescription bindDesc;
	std::vector<VkVertexInputAttributeDescription> attribDescs;


	template<class last>
	void IterThruArgs(void* allocData,int index,unsigned int vertDex,last* lastArg){
		memcpy((void*)(size_t(allocData) + attribDescs[index].offset + (bindDesc.stride * vertDex)), (void*)(size_t(lastArg) + sizeof(last) * vertDex), sizeof(last));
	}
	template< class next, class ...argsT>
	void IterThruArgs(void* allocData,int index,unsigned int vertDex,next* nextArg,argsT*... data){
		memcpy((void*)(size_t(allocData) + attribDescs[index].offset + (bindDesc.stride * vertDex)), (void*)(size_t(nextArg) + sizeof(next) * vertDex), sizeof(next));
		IterThruArgs(allocData,index+1,vertDex,data...);
	}
	template<class ...argsT>
	void* FormatVertexBuffer(unsigned int mask,argsT*... data){
		//(x[],y[],z[]) -> data
		//(x[0],y[0],z[0]),
		//(x[1],y[1],z[1])
		void* allocData = malloc(memSize);
		for(unsigned int i = 0;i < vertexNum;i++){
			IterThruArgs(allocData,0,i,data...);
		}
		return allocData;
	}

	//to write use:
	void MapData(void** data);//then
	void UnMapData();
	//OR:
	template<class ...argsT>
	void WriteData(unsigned int mask,argsT*... data){
		void* formatData = FormatVertexBuffer(mask,data...);
		void* mapped;
		MapData(&mapped);
		memcpy(mapped,formatData,memSize);
		UnMapData();
		free(formatData);
	}

	~VertexBuffer();
};


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

	unsigned int vertNum;
	std::list<VertexBuffer*> vertBuffs;

	Shader(Device* device,ShaderGroup* shaderGroup,const char* vertexShader,const char* fragmentShader);
	~Shader();
	void RegisterVertexBuffer(VertexBuffer* buff);
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
