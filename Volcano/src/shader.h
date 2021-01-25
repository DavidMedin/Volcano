#pragma once
#include <vector>
#include <list>

#undef SPV_REVISION
#include "spirv_reflect.h"
#include "shaderc/shaderc.hpp"
#undef SPV_REVISION

#include "renderpass.h"
#include "globalVulkan.h"
#include "commandpool.h"
//#include "swapchain.h"
#define MAX_FRAMES_IN_FLIGHT 4//num of timing objects. I.e. number of frames you can draw/present at the same time

void CreateFramebuffers(VkDevice device, VkRenderPass render, VkImageView* imageViews, unsigned int imageCount, VkExtent2D extent, std::vector<VkFramebuffer>* framebuffIn);


struct VertexBuffer;
struct SwapChain;
struct Shader;
struct DrawCmdGroup;
struct Window;
struct ID;
struct DrawInput;//see below
struct DrawTarget{
	Shader* shad;
	SwapChain* swapchain;
	std::shared_ptr<RenderPass> renderpass;
	// VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> frames;

	std::list<DrawInput*> inputs;
	~DrawTarget();
};
struct DrawInput{
	DrawTarget* targ;
	//input descriptions
	std::list<ID*> inputDescs;
	
	VkPipeline pipeline;
	std::list<DrawCmdGroup*> cmds;
	bool IDEquals(std::vector<ID*> ids);
};

struct ShaderMod {
	VkShaderModule mod;
	std::vector<uint32_t> code;
};
struct Shader{
	Device* device;
	VkPipelineLayout pipelineLayout;

	//index 0:vertex 1:frag 2:geom -- more in the future
	ShaderMod* shadMods;
	unsigned int shadModCount;
	ShaderGroup* group;

	VkCommandPool cmdPool;
	std::list<DrawTarget*> drawTargs;

	std::vector<VkDescriptorSetLayout> descriptorSetLayout;

	std::vector<SpvReflectInterfaceVariable*> inputVars;
	// std::vector<SpvReflectDescriptorSet*> descriptorSets;
	SpvReflectShaderModule mod;

	Shader(ShaderGroup* shaderGroup,const char* vertexShader,const char* fragmentShader);
	// Shader(std::initializer_list<ID*> ids,ShaderGroup* shaderGroup,const char* vertexShader,const char* fragmentShader);
	Shader(ShaderGroup* shaderGroup,const char* glslShader);
	// Shader(std::initializer_list<ID*> ids,ShaderGroup* shaderGroup,const char* glslShader);
	~Shader();

	void RegisterSwapChain(SwapChain* swap);
	void DestroySwapChain(SwapChain* swap);
	void DrawFrame(Window* window);
	bool ContainsSwap(SwapChain*);
	bool CompatibleID(std::vector<ID*> ids);
};


int ReadTheFile(const char* path, char** buff, unsigned int* buffSize);
VkPipelineLayout CreatePipeLayout(Device* device,std::vector<VkDescriptorSetLayout>* setLayouts);
VkPipeline CreateGraphicsPipeline(Device* device, VkPipelineLayout layout, VkRenderPass renderPass, VkExtent2D viewExtent, ShaderMod* mods,unsigned int modCount,std::list<ID*> inputDescs);

VkShaderModule CreateShaderModule(Device* device, char* code, unsigned int codeSize);
void DestroyShader(Device* device, Shader* shad);
void DestroyShaderModule();//need to be made