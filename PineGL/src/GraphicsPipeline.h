#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

int ReadTheFile(const char* path,char** buff,unsigned int* buffSize);
int CreateGraphicsPipeline(VkDevice device);
