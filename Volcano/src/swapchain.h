#pragma once
#include <algorithm>
#include <list>

#include "devices.h"
#include "globalVulkan.h"

struct Shader;
struct SwapChain {
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

	SwapChain(Device* device, VkSurfaceKHR surface);
	~SwapChain();
	void Recreate();
	VkFormat GetFormat();
};