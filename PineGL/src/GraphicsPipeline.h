#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "errorCatch.h"

int ReadTheFile(const char* path,char** buff,unsigned int* buffSize);
int CreateRenderPass(VkDevice device,VkSurfaceFormatKHR* format,VkRenderPass* renderPass);
int CreateGraphicsPipeline(VkDevice device,VkRenderPass renderPass,VkExtent2D viewExtent,VkPipelineLayout* pipelineLayout,VkPipeline* graphicsPipeline);

