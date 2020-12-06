#pragma once
#include <vector>
#include <list>
#include <cstdint>

#include "globalVulkan.h"
#include "devices.h"
#include "errorCatch.h"


struct DrawObj;

VkCommandPool CreateCommandPool(Device* device,int flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

std::vector<VkCommandBuffer>* CreateCommandBuffers(Device* device,VkCommandPool commandPool,unsigned int commandBuffCount);
// void CreateCommandBuffers(Device* device,VkCommandPool commandPool,unsigned int cmdBufferCount,VkCommandBuffer* cmdBuffers);

void FillCommandBuffers(VkExtent2D swapChainExtent,std::vector<VkFramebuffer>* frameBuffs,VkPipeline graphicsPipeline,VkRenderPass renderPass,DrawObj* drawObj,std::vector<VkCommandBuffer>* cmdBuffs);

int CreateSemaphore(VkDevice device, VkSemaphore* semaphore);
int CreateFence(VkDevice device,VkFence* fence);