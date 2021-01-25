#pragma once
#include <algorithm>
#include <list>
#include <vector>

#include <spirv_cross/spirv_reflect.hpp>

#include "devices.h"
#include "globalVulkan.h"

struct Shader;
struct DrawCmdGroup;
struct SwapChain {
	VkDescriptorPool descriptorPool;

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

	unsigned int imageIndex;
	unsigned int nextTimeObj;
	std::vector<VkFence> imageFence;

	std::vector<VkSemaphore> available;
	std::vector<VkSemaphore> presentable;
	std::vector<VkFence> fences;

	std::vector<std::vector<VkCommandBuffer>*> drawCmds;
	unsigned int nextDrawCmd;
	bool justDrawn;

	SwapChain(Device* device, VkSurfaceKHR surface);
	~SwapChain();
	void Recreate();
	VkFormat GetFormat();

	void AddDrawCmd(DrawCmdGroup* cmd);
	void DrawFrame();
};