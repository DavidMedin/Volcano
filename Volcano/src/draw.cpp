#include "draw.h"
#include "vertexbuffer.h"
#include "shader.h"
#include "swapchain.h"

DrawObj::DrawObj(std::initializer_list<VertexBuffer*> vertBuffs,IndexBuffer* index){
    this->vertBuffs.resize(vertBuffs.size());
    for(auto vert: vertBuffs){
        this->vertBuffs.push_back(vert);
    }
    indexBuff = index;   
}
void DrawObj::RegisterShader(Shader* shad){
    DrawShader* tmpDraw = new DrawShader;
    tmpDraw->shad = shad;
    for(auto swap:registeredSwaps){
        RegisterSwapChain(swap);
    }
    drawShads.push_back(tmpDraw);
}
void DrawObj::RegisterSwapChain(SwapChain* swap){
    for(auto draw: drawShads){
        for(auto targs:draw->shad->commands){
            if(targs->swapchain == swap){
                draw->drawTargs.push_back(targs);
            }
        }
    }
}