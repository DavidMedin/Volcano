#pragma once
#include "globalVulkan.h"
#include "window.h"
#include "framebuffer.h"
typedef VkRenderPass Renderpass;
void CreateRenderPass(Window* win, Device* device,VkRenderPass* renderPass);