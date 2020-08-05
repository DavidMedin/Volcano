#pragma once
#include "globalVulkan.h"
#include <vulkan/vulkan.h>

int CreateFramebuffers(VkDevice device,VkRenderPass renderPass,VkExtent2D swapExtent,VkImageView* views,unsigned int viewCount,VkFramebuffer** framebuffers,unsigned int* framebufferCount);