#include "draw.h"
#include "vertexbuffer.h"
#include "shader.h"
#include "window.h"
#include "descriptorSet.h"


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
        // std::list<ID*> ids;
        // for(auto vert : vertBuffs){
        //     ids.push_back(vert->inDesc);
        // }
        DrawTarget* tmpTarg;
        DrawInput* tmpInput;
        for(auto targ:shad->drawTargs){
            if(targ->swapchain == swap){
                tmpTarg = targ;
                bool found = false;
                for(auto input:targ->inputs){
                    if(input->IDEquals(ids)){
                        drawInputs.push_back(input);
                        tmpInput = input;
                        found = true;
                        break;
                    }
                }
                if(found == false){
                    //Create a new DrawInput
                    DrawInput* in = new DrawInput;
                    in->targ = targ;
                    for(auto id : ids){
                        in->inputDescs.push_back(id);
                    }
                    in->pipeline = CreateGraphicsPipeline(device,shad->pipelineLayout,targ->renderpass->renderpass,targ->swapchain->swapExtent,shad->shadMods,shad->shadModCount,in->inputDescs);
                    tmpInput = in;
                    targ->inputs.push_back(in);
                    drawInputs.push_back(in);
                }
                break;
            }
        }
        //create/fill cmd buffers
        DrawCmdGroup* tmpDrawGroup = new DrawCmdGroup;
        std::vector<VkCommandBuffer>* tmpDrawBuffs = CreateCommandBuffers(shad->device, shad->cmdPool,swap->imageCount);
        std::vector<VkCommandBuffer>* tmpClearBuffs = CreateCommandBuffers(shad->device, shad->cmdPool,swap->imageCount);
        tmpDrawGroup->mainDraw = tmpDrawBuffs;
        tmpDrawGroup->clearDraw = tmpClearBuffs;
        // drawCmds.push_back(tmpCmdBuffs);
        FillCommandBuffers(swap->swapExtent, &tmpTarg->frames,tmpInput->pipeline,tmpTarg->renderpass->renderpass,this,tmpDrawBuffs);
        FillCommandBuffers(swap->swapExtent, &tmpTarg->frames,tmpInput->pipeline, tmpTarg->renderpass->clearpass,this,tmpClearBuffs);

        for(auto targ:shad->drawTargs){
            if(targ->swapchain == swap){
                tmpInput->cmds.push_back(tmpDrawGroup);
            }
        }

        tmpDrawGroup->draw = this;
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
        
        ids.push_back(vert->inDesc);
        this->vertBuffs[i] = vert;
        i++;
    }
    if(!shad->CompatibleID(ids)){
        Error("Vertex buffers given to DrawObj() are not compatable with the shader!\n");
    }
    sets.resize(shad->descriptorSetLayouts.size());
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
        ids.push_back(vert->inDesc);
        this->vertBuffs[i] = vert;
        i++;
    }
    if(!shad->CompatibleID(ids)){
        Error("Vertex buffers given to DrawObj() are not compatable with the shader!\n");
    }
    sets.resize(shad->descriptorSetLayouts.size());
    vertNum = tmpVertNum;
    indexBuff = nullptr;
    this->shad = shad;
    device = GetCurrentDevice();
}
void DrawObj::BindSet(DescriptorSet* set){
    //add set to list
    sets.push_back(set);
    //recreate command buffers
    for(auto cmdGroup : drawCmds){
        RecalculateCmdBuffs(cmdGroup);
    }
}
void DrawObj::QueueDraw(Window* win){
    SwapChain* swap = win->swapchain;
    //check if swapchain is registered in this shader
    if(!DrawContains(this,swap)) RegisterSwapChain(swap);

    unsigned int i = 0;
    for (auto input : drawInputs) {
        if (input->targ->swapchain == swap) {
            swap->AddDrawCmd(drawCmds[i]);
            return;
        }
        i++;
    }
}
void DrawObj::RecalculateCmdBuffs(DrawCmdGroup* group){
    unsigned int i = 0;
    for(auto g : drawCmds){
        if(g == group){
            //recalculate this group
            //create/fill cmd buffers
            std::vector<VkCommandBuffer>* tmpDrawBuffs = CreateCommandBuffers(shad->device, shad->cmdPool,registeredSwaps[i]->imageCount);
            std::vector<VkCommandBuffer>* tmpClearBuffs = CreateCommandBuffers(shad->device, shad->cmdPool,registeredSwaps[i]->imageCount);
            group->mainDraw = tmpDrawBuffs;
            group->clearDraw = tmpClearBuffs;
            // drawCmds.push_back(tmpCmdBuffs);
            FillCommandBuffers(registeredSwaps[i]->swapExtent, &drawInputs[i]->targ->frames,drawInputs[i]->pipeline, drawInputs[i]->targ->renderpass->renderpass,this,tmpDrawBuffs);
            FillCommandBuffers(registeredSwaps[i]->swapExtent, &drawInputs[i]->targ->frames,drawInputs[i]->pipeline, drawInputs[i]->targ->renderpass->clearpass,this,tmpClearBuffs);
        }
        i++;
    }
}