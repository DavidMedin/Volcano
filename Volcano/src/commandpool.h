#pragma once
#include <vector>

#include "globalVulkan.h"
#include "devices.h"
#include "errorCatch.h"

#include <vulkan/vulkan.h>

VkCommandPool CreateCommandPool(VkDevice device,QueueFamilyIndex* indices);

std::vector<VkCommandBuffer>* CreateCommandBuffers(Device* device,VkCommandPool commandPool,unsigned int commandBuffCount);
// void CreateCommandBuffers(Device* device,VkCommandPool commandPool,unsigned int cmdBufferCount,VkCommandBuffer* cmdBuffers);

void FillCommandBuffers(VkExtent2D swapChainExtent,std::vector<VkFramebuffer>* frameBuffs,VkPipeline graphicsPipeline,VkRenderPass renderPass,std::vector<VkCommandBuffer>* cmdBuffs);

int CreateSemaphore(VkDevice device, VkSemaphore* semaphore);
int CreateFence(VkDevice device,VkFence* fence);