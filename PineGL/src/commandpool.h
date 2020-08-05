#pragma once
#include "globalVulkan.h"
#include "devices.h"

#include <vulkan/vulkan.h>

int CreateCommandPool(VkDevice device,QueueFamilyIndex* indices,VkCommandPool* commandPool);
int CreateCommandBuffers(VkDevice device,VkCommandPool commandPool,unsigned int cmdBufferCount,VkCommandBuffer** cmdBuffers);
int FillCommandBuffers(VkExtent2D swapChainExtent,VkFramebuffer* framebuffers,unsigned int framebufferCount,VkPipeline graphicsPipeline,VkRenderPass renderPass,unsigned int cmdbufferCount,VkCommandBuffer* cmdbuffers);

int CreateSemaphore(VkDevice device, VkSemaphore* semaphore);
int CreateFence(VkDevice device,VkFence* fence);