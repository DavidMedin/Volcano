#pragma once
#include "globalVulkan.h"
#include "window.h"
#include "framebuffer.h"
typedef VkRenderPass Renderpass;
void CreateRenderPass(Window* win,unsigned int windowCount, Device* device,VkRenderPass* renderPass);