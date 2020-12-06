#include "draw.h"
#include "vertexbuffer.h"
#include "shader.h"
#include "window.h"

bool DrawContains(DrawObj* obj, SwapChain* swap){
    for(auto tmpSwap:obj->registeredSwaps){
        if(tmpSwap == swap){
            return true;
        }
    }
    return false;
}

void DrawObj::RegisterSwapChain(SwapChain* swap){
    if(!DrawContains(this,swap)){
        if(!shad->ContainsSwap(swap)) shad->RegisterSwapChain(swap);
        DrawTarget* tmpTarg;
        for(auto targ:shad->drawTargs){
            if(targ->swapchain == swap){
                drawTargs.push_back(targ);
                tmpTarg = targ;
            }
        }
        //create/fill cmd buffers
        std::vector<VkCommandBuffer>* tmpCmdBuffs = CreateCommandBuffers(shad->device, shad->cmdPool,swap->imageCount);
        drawCmds.push_back(tmpCmdBuffs);
        FillCommandBuffers(swap->swapExtent, &tmpTarg->frames,tmpTarg->graphicsPipeline, tmpTarg->renderpass->renderpass,this,tmpCmdBuffs);
        registeredSwaps.push_back(swap);
    }
}

DrawObj::DrawObj(std::initializer_list<VertexBuffer*> vertBuffs,IndexBuffer* index,Shader* shad){
    this->vertBuffs.resize(vertBuffs.size());
    unsigned int tmpVertNum = (*vertBuffs.begin())->vertexNum;
    unsigned int i = 0;
    for(auto vert: vertBuffs){
        if(tmpVertNum != vert->vertexNum){
            Error("Vertex Buffer vertex number mismatch!\n");
            return;
        }
        this->vertBuffs[i] = vert;
        i++;
    }
    vertNum = tmpVertNum;
    indexBuff = index;
    this->shad = shad;
    device = GetCurrentDevice();
}
DrawObj::DrawObj(std::initializer_list<VertexBuffer*> vertBuffs,Shader* shad){
    this->vertBuffs.resize(vertBuffs.size());
    unsigned int tmpVertNum = (*vertBuffs.begin())->vertexNum;
    unsigned int i = 0;
    for(auto vert: vertBuffs){
        if(tmpVertNum != vert->vertexNum){
            Error("Vertex Buffer vertex number mismatch!\n");
            return;
        }
        this->vertBuffs[i] = vert;
        i++;
    }
    vertNum = tmpVertNum;
    this->shad = shad;
    device = GetCurrentDevice();
}
void DrawObj::Draw(Window* win){
    SwapChain* swap = win->swapchain;
    unsigned int nextFrame = swap->nextTimeObj;
    
    //check if swapchain is registered in this shader
    if(!DrawContains(this,swap)) RegisterSwapChain(swap);

    unsigned int i = 0;
    for (auto targetCommands : drawTargs) {
        if (targetCommands->swapchain == swap) {
            vkWaitForFences(device->device, 1, &swap->fences[nextFrame], VK_TRUE, UINT64_MAX);

            VkResult rez = vkAcquireNextImageKHR(device->device, swap->swapChain, UINT64_MAX, swap->available[nextFrame], nullptr, &swap->imageIndex);

            if (swap->imageFence[swap->imageIndex] != VK_NULL_HANDLE) {
                vkWaitForFences(device->device, 1, &swap->imageFence[swap->imageIndex], VK_TRUE, UINT64_MAX);
            }
            vkResetFences(device->device, 1, &swap->fences[nextFrame]);
            swap->imageFence[swap->imageIndex] = swap->fences[nextFrame];


            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &swap->available[nextFrame];
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &(*drawCmds[i])[swap->imageIndex];
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &swap->presentable[nextFrame];
            if (vkQueueSubmit(device->queues[0], 1, &submitInfo, swap->fences[nextFrame]) != VK_SUCCESS) {
                Error("Couldn't send command buffer to graphics queue\n");
            }
            return;
        }
        i++;
    }
}