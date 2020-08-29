#pragma once
#include "globalVulkan.h"
#include <vulkan/vulkan.h>
#include "errorCatch.h"
#include "window.h"
void CreateFramebuffers(VkDevice device,VkRenderPass* renderpasses,unsigned int renderCount,Window win);