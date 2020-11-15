#include "commandpool.h"
#include "shader.h"
#include "vertexbuffer.h"
VkCommandPool CreateCommandPool(VkDevice device,QueueFamilyIndex* indices){
    VkCommandPool pool;
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices->graphics;
    if(vkCreateCommandPool(device,&poolInfo,NULL,&pool) != VK_SUCCESS){
        Error("   command pool create failed\n");
        return nullptr;
    }
    return pool;
}

//command pool and cmdbuffercount are input
//cmdBuffs is a null pointer to an array
std::vector<VkCommandBuffer>* CreateCommandBuffers(Device* device,VkCommandPool commandPool,unsigned int commandBuffCount){
    //allocate mem
    // *cmdBuffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer)*cmdBufferCount);
    std::vector<VkCommandBuffer>* commandBuffs = new std::vector<VkCommandBuffer>(commandBuffCount);
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//most simple use case
    allocInfo.commandBufferCount = commandBuffCount;

    if(vkAllocateCommandBuffers(device->device,&allocInfo,commandBuffs->data()) != VK_SUCCESS){
        Error("    alloc command buffers create failed\n");
    }
    return commandBuffs;
}
void CreateCommandBuffers(Device* device,VkCommandPool commandPool,unsigned int cmdBufferCount,VkCommandBuffer* cmdBuffers){
    //allocate mem
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//most simple use case
    allocInfo.commandBufferCount = cmdBufferCount;

    if(vkAllocateCommandBuffers(device->device,&allocInfo,cmdBuffers) != VK_SUCCESS){
        Error("    alloc command buffers create failed\n");
    }
}


void FillCommandBuffers(VkExtent2D swapChainExtent,std::vector<VkFramebuffer>* frameBuffs,VkPipeline graphicsPipeline,VkRenderPass renderPass,Shader* shad,std::vector<VkCommandBuffer>* cmdBuffs){
    std::vector<VkCommandBuffer> deCmdBuffs = *cmdBuffs;
    std::vector<VkFramebuffer> deFrameBuffs = *frameBuffs;
    if(deFrameBuffs.size() != deCmdBuffs.size()){
        Error("not an equal number of framebuffers and commandbuffers\n");
        return;
    }
    for(unsigned int i = 0;i < deCmdBuffs.size();i++){
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //begin recording this command buffer
        if(vkBeginCommandBuffer(deCmdBuffs[i],&beginInfo) != VK_SUCCESS){
            Error("    begin command buffer : %d failed\n",i);
            return;
        }
        //the only commmands we need to record/do is to start the render pass
        VkRenderPassBeginInfo renderStartInfo = {};
        renderStartInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderStartInfo.renderPass = renderPass;
        renderStartInfo.framebuffer = deFrameBuffs[i];
        VkOffset2D offset = {0,0};
        renderStartInfo.renderArea.offset = offset;
        renderStartInfo.renderArea.extent = swapChainExtent;

        VkClearValue clearColor = {0.0f,0.0f,0.0f,1.0f};
        renderStartInfo.clearValueCount = 1;
        renderStartInfo.pClearValues = &clearColor;
        //this is the only thing that is being 'recorded' here
        vkCmdBeginRenderPass(deCmdBuffs[i],&renderStartInfo,VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(deCmdBuffs[i],VK_PIPELINE_BIND_POINT_GRAPHICS,graphicsPipeline);

        // VkBuffer* vertBuffArray = (VkBuffer*)malloc(sizeof(VkBuffer)*shad->vertBuffs.size());
        // VkDeviceSize* offsets = (VkDeviceSize*)calloc(1,sizeof(VkDeviceSize)*shad->vertBuffs.size());//should be zero?
        VkBuffer vertBuffArray[1] = {};
        VkDeviceSize offsets[1] = {0};
        unsigned int index = 0;
        for(auto buff : shad->vertBuffs){
            vertBuffArray[index] = buff->buff;
            index++;
        }
        vkCmdBindVertexBuffers(deCmdBuffs[i],0,shad->vertBuffs.size(),vertBuffArray,offsets);
        vkCmdDraw(deCmdBuffs[i],shad->vertNum,1,0,0);
        //end the cmd recording
        vkCmdEndRenderPass(deCmdBuffs[i]);
        if(vkEndCommandBuffer(deCmdBuffs[i]) != VK_SUCCESS){
            Error("end command buffer : %d failed\n",i);
            return;
        }
        // free(vertBuffArray);
        // free(offsets);
    }
}


int CreateSemaphore(VkDevice device, VkSemaphore* semaphore){
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if(vkCreateSemaphore(device,&semaphoreInfo,NULL,semaphore) != VK_SUCCESS){
        Error("    semaphore create failed\n");
        return 0;
    }
    return 1;
}

int CreateFence(VkDevice device,VkFence* fence){
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if(vkCreateFence(device,&fenceInfo,NULL,fence) != VK_SUCCESS){
        Error("    fence create failed\n");
        return 0;
    }
    return 1;
}