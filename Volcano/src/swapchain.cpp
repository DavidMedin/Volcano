#include "swapchain.h"
#include "shader.h"
#include "draw.h"
extern std::list<SwapChain*> swapList;
extern std::list<Shader*> shadList;


//device,imageCount,images,and imageFormat is input
//imageViews is an output arguments
int CreateImageViews(VkDevice device, unsigned int imageCount, VkImage* images, VkSurfaceFormatKHR* imageFormat, VkImageView** imageViews) {
    if (imageCount > 0) {
        *imageViews = (VkImageView*)malloc(sizeof(VkImageView) * imageCount);
        for (unsigned int i = 0; i < imageCount; i++) {
            //create the creation struct
            VkImageViewCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageInfo.image = images[i];
            //we are dealing with 2d things here
            imageInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageInfo.format = imageFormat->format;

            imageInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            //describes image access and use
            imageInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;//useed for color?
            imageInfo.subresourceRange.baseMipLevel = 0;
            imageInfo.subresourceRange.levelCount = 1;
            imageInfo.subresourceRange.baseArrayLayer = 0;
            imageInfo.subresourceRange.layerCount = 1;
            if (vkCreateImageView(device, &imageInfo, NULL, &(*imageViews)[i]) != VK_SUCCESS) {
                Error("Couldn't create a imageView!\n");
                return 0;
            }
        }
    }
    else {
        Error("CreateImageViews was given 0 as an imageCount! that is wrong.\n");
        return 0;
    }
    return 1;
}

//returns the index into formats that should be used
unsigned int Swap_GetBestSurfaceFormat(SwapChainSupportDetails* dets) {
    for (unsigned int i = 0; i < dets->formatCount; i++) {
        if (dets->formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && dets->formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return i;
        }
    }
    Error("the ideal colorspace wasn't provided in formats!\n");
    return 0;
}

unsigned int Swap_GetBestPresentMode(SwapChainSupportDetails* dets) {
    unsigned int fifo = 0;
    for (unsigned int i = 0; i < dets->presentCount; i++) {
        switch (dets->presentModes[i]) {
        case VK_PRESENT_MODE_MAILBOX_KHR:
            return i;
        case VK_PRESENT_MODE_FIFO_KHR:
            fifo = i;
        }
        return fifo;
    }
    printf("no triple buffer present mode, falling back to double buffer\n");
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swap_GetBestSwapExtent(SwapChainSupportDetails* dets, unsigned int targWidth, unsigned int targHeight) {
    if (dets->capabilities.currentExtent.width != UINT32_MAX) {
        return dets->capabilities.currentExtent;
    }
    else {
        VkExtent2D actualExtent = { targWidth,targHeight };
        actualExtent.width = std::min(std::max(dets->capabilities.minImageExtent.width, actualExtent.width), dets->capabilities.maxImageExtent.width);
        actualExtent.height = std::min(std::max(dets->capabilities.minImageExtent.height, actualExtent.height), dets->capabilities.maxImageExtent.height);
        return actualExtent;
    }
}


void RecreateSwapchain(Device* devDets, VkSurfaceKHR surface, SwapChain* swap) {
    VkDevice device = devDets->device;
    QueueFamilyIndex* famz = &devDets->families;
    // renderpassCount = 0;
    if (!IsDeviceCompatible(swap->device->phyDev, surface, swap->device->phyProps, &swap->device->families, &swap->swapDets)) {
        Error("This surface doesn't comply with the picked device. wack\n");
    }
    swap->chosenFormat = Swap_GetBestSurfaceFormat(&swap->swapDets);
    swap->chosenPresent = Swap_GetBestPresentMode(&swap->swapDets);
    VkSurfaceFormatKHR* format = &(swap->swapDets.formats[swap->chosenFormat]);
    VkPresentModeKHR present = swap->swapDets.presentModes[swap->chosenPresent];
    VkExtent2D extent = Swap_GetBestSwapExtent(&swap->swapDets, WIDTH, HEIGHT);

    swap->imageCount = swap->swapDets.capabilities.minImageCount + 1;
    //if maximage is 0, unlimited images; otherwise get the min of the two
    if (swap->swapDets.capabilities.maxImageCount > 0 && swap->imageCount > swap->swapDets.capabilities.maxImageCount) {
        swap->imageCount = swap->swapDets.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR swapInfo = {};
    swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapInfo.surface = surface;
    swapInfo.minImageCount = swap->imageCount;
    swapInfo.imageFormat = format->format;
    swapInfo.imageColorSpace = format->colorSpace;
    swapInfo.imageExtent = extent;
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    //taking the availability of .graphics and .presentation for granted
    if (famz->presentation == famz->graphics) {
        swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//only one family can push to the swapchain
        swapInfo.queueFamilyIndexCount = 0;
        swapInfo.pQueueFamilyIndices = NULL;
    }
    else {
        swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//more than one can push, need to juggle permissions tho
        swapInfo.queueFamilyIndexCount = 2;
        unsigned int indices[] = { famz->graphics,famz->presentation };
        swapInfo.pQueueFamilyIndices = indices;
    }

    swapInfo.preTransform = swap->swapDets.capabilities.currentTransform; // no applied transform
    swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //no alpha blending with other windows

    swapInfo.presentMode = present;
    swapInfo.clipped = VK_TRUE;//don't care about pixels that are obstructed by other windows

    swapInfo.oldSwapchain = VK_NULL_HANDLE;//for when we need to change the swapchain (like when we need to resize the window)

    if (vkCreateSwapchainKHR(device, &swapInfo, NULL, &swap->swapChain) != VK_SUCCESS) {
        Error("couldn't create a swap chain! oh no boo hoo.\n");
    }

    swap->swapExtent = extent;
}


// int CreateSwapChain(Device* devDets,VkSurfaceKHR surface, SwapChain* swapchain){
SwapChain::SwapChain(Device* devDets, VkSurfaceKHR surface) {
    this->windowResized = false;
    this->device = devDets;
    this->surface = surface;
    this->nextTimeObj = 0;
    nextDrawCmd = 0;
    justDrawn = false;
    RecreateSwapchain(devDets, surface, this);
    //get the VkImages
    imageCount = 0;
    vkGetSwapchainImagesKHR(device->device, swapChain, &imageCount, NULL);
    images = (VkImage*)malloc(sizeof(VkImage) * imageCount);
    vkGetSwapchainImagesKHR(device->device, swapChain, &imageCount, images);
    if (imageCount == 0) {
        Error("the swap chain has zero images!\n");
    }

    if (!CreateImageViews(device->device, imageCount, images, &swapDets.formats[chosenFormat], &imageViews)) {
        Error("Couldn't create image views!\n");
    }

    imageFence.resize(imageCount, VK_NULL_HANDLE);
    available.resize(MAX_FRAMES_IN_FLIGHT);
    presentable.resize(MAX_FRAMES_IN_FLIGHT);
    fences.resize(MAX_FRAMES_IN_FLIGHT);
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CreateSemaphore(device->device, &(available[i]));
        CreateSemaphore(device->device, &(presentable[i]));
        CreateFence(device->device, &(fences[i]));
    }

    //deprecated because of https://trello.com/c/h9QYwvHr/38-draw-separate-objects
    // for (auto shad : shadList) {
    //     shad->RegisterSwapChain(this);
    // }
}
SwapChain::~SwapChain() {
    for (unsigned int i = 0; i < imageCount; i++) {
        vkDestroyImageView(device->device, imageViews[i], NULL);
    }
    free(imageViews);
    for (auto shad : shadList) {
        shad->DestroySwapChain(this);
    }
    vkDestroySwapchainKHR(device->device, swapChain, NULL);
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device->device, available[i], NULL);
        vkDestroySemaphore(device->device, presentable[i], NULL);
        vkDestroyFence(device->device, fences[i], NULL);
    }
}

void SwapChain::AddDrawCmd(DrawCmdGroup* cmd){
    if(nextDrawCmd+1 > drawCmds.size()) drawCmds.resize(nextDrawCmd+1);
    if(nextDrawCmd == 0)
        drawCmds[nextDrawCmd] = cmd->clearDraw;
    else
        drawCmds[nextDrawCmd] = cmd->mainDraw;
    nextDrawCmd++;
}

void SwapChain::DrawFrame(){
    vkWaitForFences(device->device, 1, &fences[nextTimeObj], VK_TRUE, UINT64_MAX);
    VkResult rez = vkAcquireNextImageKHR(device->device, swapChain, UINT64_MAX, available[nextTimeObj], nullptr, &imageIndex);
    if (imageFence[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(device->device, 1, &imageFence[imageIndex], VK_TRUE, UINT64_MAX);
    }
    vkResetFences(device->device, 1, &fences[nextTimeObj]);
    imageFence[imageIndex] = fences[nextTimeObj];
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &available[nextTimeObj];
    submitInfo.pWaitDstStageMask = waitStages;

    //create vector of cmd buffs
    std::vector<VkCommandBuffer> cmds;
    for(auto cmdList : drawCmds){
        if(cmdList->size() != 0)
        cmds.push_back((*cmdList)[imageIndex]);
    }
    submitInfo.commandBufferCount = (unsigned int)cmds.size();
    submitInfo.pCommandBuffers = cmds.data();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &presentable[nextTimeObj];
    if (vkQueueSubmit(device->queues[0], 1, &submitInfo, fences[nextTimeObj]) != VK_SUCCESS) {
        Error("Couldn't send command buffer to graphics queue\n");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &presentable[nextTimeObj];
    presentInfo.swapchainCount = 1;//whaaaatttt?
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;
    rez = vkQueuePresentKHR(device->queues[1], &presentInfo);//rewriting rez
    if (rez == VK_ERROR_OUT_OF_DATE_KHR || rez == VK_SUBOPTIMAL_KHR || windowResized) {
        windowResized = false;
        Recreate();
    }
    else if (rez != VK_SUCCESS) {
        Error("Failed to acquire a swapchain image\n");
    }
    // vkQueueWaitIdle(device->queues[1]);
    nextTimeObj = (nextTimeObj + 1) % MAX_FRAMES_IN_FLIGHT;

    drawCmds.clear();
    nextDrawCmd = 0;
}


void SwapChain::Recreate() {
    //my copy pasta is esquisite
    int width = 0, height = 0;
    glfwGetFramebufferSize(win, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(win, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device->device);

    //create new swapchain
    vkDestroySwapchainKHR(device->device, swapChain, NULL);
    RecreateSwapchain(device, surface, this);

    unsigned int oldCount = imageCount;
    vkGetSwapchainImagesKHR(device->device, swapChain, &imageCount, NULL);
    if (imageCount != oldCount) {
        free(images);
        images = (VkImage*)malloc(sizeof(VkImage) * imageCount);
    }
    vkGetSwapchainImagesKHR(device->device, swapChain, &imageCount, images);
    if (imageCount == 0) {
        Error("the swap chain has zero images!\n");
    }
    //destroy image views
    for (unsigned int i = 0; i < oldCount; i++) {
        vkDestroyImageView(device->device, imageViews[i], NULL);
    }
    if (oldCount != imageCount) {
        free(imageViews);
        imageViews = (VkImageView*)malloc(sizeof(VkImageView) * imageCount);
    }
    //create image views
    if (!CreateImageViews(device->device, imageCount, images, &swapDets.formats[chosenFormat], &imageViews)) {
        Error("Couldn't create image views!\n");
    }
    for (auto shad : shadList) {
        for (auto command : shad->drawTargs) {
            if (command->swapchain == this) {
                command->renderpass = CreateRenderpass(command->swapchain->GetFormat(), device, shad->group);//this should get an existing renderpass, dunno tho

                vkDestroyPipeline(device->device, command->graphicsPipeline, NULL);
                command->graphicsPipeline = CreateGraphicsPipeline(device, shad->pipelineLayout, command->renderpass->renderpass, swapExtent, shad);
                //delete/create new framebuffers
                for (auto framebuffer : command->frames) {
                    vkDestroyFramebuffer(device->device, framebuffer, NULL);
                }
                CreateFramebuffers(device->device, command->renderpass->renderpass, imageViews, imageCount, swapExtent, &command->frames);

                //fill the command buffers
                // FillCommandBuffers(swapExtent, &command->frames, command->graphicsPipeline, command->renderpass->renderpass, shad, command->drawCommands);

            }
        }
    }
}


VkFormat SwapChain::GetFormat() {
    return swapDets.formats[chosenFormat].format;
}