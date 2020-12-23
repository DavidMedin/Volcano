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
        DrawCmdGroup* tmpDrawGroup = new DrawCmdGroup;
        std::vector<VkCommandBuffer>* tmpDrawBuffs = CreateCommandBuffers(shad->device, shad->cmdPool,swap->imageCount);
        std::vector<VkCommandBuffer>* tmpClearBuffs = CreateCommandBuffers(shad->device, shad->cmdPool,swap->imageCount);
        tmpDrawGroup->mainDraw = tmpDrawBuffs;
        tmpDrawGroup->clearDraw = tmpClearBuffs;
        // drawCmds.push_back(tmpCmdBuffs);
        FillCommandBuffers(swap->swapExtent, &tmpTarg->frames,tmpTarg->graphicsPipeline, tmpTarg->renderpass->renderpass,this,tmpDrawBuffs);
        FillCommandBuffers(swap->swapExtent, &tmpTarg->frames,tmpTarg->graphicsPipeline, tmpTarg->renderpass->clearpass,this,tmpClearBuffs);
        drawCmds.push_back(tmpDrawGroup);
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

    //check that all the vertex buffers have the same vert num
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
void DrawObj::QueueDraw(Window* win){
    SwapChain* swap = win->swapchain;
    //check if swapchain is registered in this shader
    if(!DrawContains(this,swap)) RegisterSwapChain(swap);

    unsigned int i = 0;
    for (auto targetCommands : drawTargs) {
        if (targetCommands->swapchain == swap) {
            swap->AddDrawCmd(drawCmds[i]);
            return;
        }
        i++;
    }
}