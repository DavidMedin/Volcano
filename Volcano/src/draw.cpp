#include "draw.h"
#include "vertexbuffer.h"
#include "shader.h"
#include "swapchain.h"

bool DrawShaderContains(DrawShader* shad, SwapChain* swap){
    for(auto draw:shad->drawTargs){
        if(draw->swapchain == swap){
            return true;
        }
    }
    return false;
}

void DrawShader::RegisterSwapChain(SwapChain* swap){
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
    FillCommandBuffers(swap->swapExtent, &tmpTarg->frames,tmpTarg->graphicsPipeline, tmpTarg->renderpass->renderpass, shad,tmpCmdBuffs);
}

DrawObj::DrawObj(std::initializer_list<VertexBuffer*> vertBuffs,IndexBuffer* index){
    this->vertBuffs.resize(vertBuffs.size());
    for(auto vert: vertBuffs){
        this->vertBuffs.push_back(vert);
    }
    indexBuff = index;   
}
DrawObj::DrawObj(std::initializer_list<VertexBuffer*> vertBuffs){
    this->vertBuffs.resize(vertBuffs.size());
    for(auto vert: vertBuffs){
        this->vertBuffs.push_back(vert);
    }
}
void DrawObj::RegisterShader(Shader* shad){
    DrawShader* tmpDraw = new DrawShader;
    tmpDraw->shad = shad;
    for(auto swap:registeredSwaps){
        tmpDraw->RegisterSwapChain(swap);
    }
    drawShads.push_back(tmpDraw);
}
void DrawObj::RegisterSwapChain(SwapChain* swap){
    for(auto draw: drawShads){
        draw->RegisterSwapChain(swap);
    }
    registeredSwaps.push_back(swap);
}
void Draw(Shader* shad,SwapChain* swap){
    
}