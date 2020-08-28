#pragma once
#include "globalVulkan.h"
#include <vulkan/vulkan.h>
#include "errorCatch.h"
#include "window.h"
int CreateFramebuffers(VkDevice device,VkRenderPass renderPass,Window win,VkFramebuffer** framebuffers,unsigned int* framebufferCount);