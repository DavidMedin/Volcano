#include "commandpool.h"

int CreateCommandPool(VkDevice device,QueueFamilyIndex* indices,VkCommandPool* commandPool){
     VkCommandPoolCreateInfo poolInfo = {0};
     poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
     poolInfo.queueFamilyIndex = indices->graphics;
     if(vkCreateCommandPool(device,&poolInfo,NULL,commandPool) != VK_SUCCESS){
         Error("   command pool create failed\n");
         return 0;
     }
     return 1;
}

//command pool and cmdbuffercount are input
//cmdbuffers is a null pointer to an array
int CreateCommandBuffers(VkDevice device,VkCommandPool commandPool,unsigned int cmdBufferCount,VkCommandBuffer** cmdBuffers){
    //allocate mem
    *cmdBuffers = malloc(sizeof(VkCommandBuffer)*cmdBufferCount);

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//most simple use case
    allocInfo.commandBufferCount = cmdBufferCount;

    if(vkAllocateCommandBuffers(device,&allocInfo,*cmdBuffers) != VK_SUCCESS){
        Error("    alloc command buffers create failed\n");
        return 0;
    }
    return 1;
}

int FillCommandBuffers(VkExtent2D swapChainExtent,VkFramebuffer* framebuffers,unsigned int framebufferCount,VkPipeline graphicsPipeline,VkRenderPass renderPass,unsigned int cmdbufferCount,VkCommandBuffer* cmdbuffers){
    if(framebufferCount != cmdbufferCount){
        Error("not an equal number of framebuffers and commandbuffers\n");
        return 0;
    }
    for(unsigned int i =0;i < cmdbufferCount;i++){
        VkCommandBufferBeginInfo beginInfo = {0};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //begin recording this command buffer
        if(vkBeginCommandBuffer(cmdbuffers[i],&beginInfo) != VK_SUCCESS){
            Error("    begin command buffer : %d failed\n",i);
            return 0;
        }
        //the only commmands we need to record/do is to start the render pass
        VkRenderPassBeginInfo renderStartInfo = {0};
        renderStartInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderStartInfo.renderPass = renderPass;
        renderStartInfo.framebuffer = framebuffers[i];
        VkOffset2D offset = {0,0};
        renderStartInfo.renderArea.offset = offset;
        renderStartInfo.renderArea.extent = swapChainExtent;

        VkClearValue clearColor = {0.0f,0.0f,0.0f,1.0f};
        renderStartInfo.clearValueCount = 1;
        renderStartInfo.pClearValues = &clearColor;
        //this is the only thing that is being 'recorded' here
        vkCmdBeginRenderPass(cmdbuffers[i],&renderStartInfo,VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmdbuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS,graphicsPipeline);
        vkCmdDraw(cmdbuffers[i],3,1,0,0);
        //end the cmd recording
        vkCmdEndRenderPass(cmdbuffers[i]);
        if(vkEndCommandBuffer(cmdbuffers[i]) != VK_SUCCESS){
            Error("end command buffer : %d failed\n",i);
            return 0;
        }
    }
    return 1;
}


int CreateSemaphore(VkDevice device, VkSemaphore* semaphore){
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if(vkCreateSemaphore(device,&semaphoreInfo,NULL,semaphore) != VK_SUCCESS){
        Error("    semaphore create failed\n");
        return 0;
    }
    return 1;
}

int CreateFence(VkDevice device,VkFence* fence){
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if(vkCreateFence(device,&fenceInfo,NULL,fence) != VK_SUCCESS){
        Error("    fence create failed\n");
        return 0;
    }
    return 1;
}