#pragma once
#include "globalVulkan.h"
#include "window.h"
typedef VkRenderPass Renderpass;
void CreateRenderPass(Window* win, Device* device,Renderpass* renderPass);