#pragma once
#include <vector>
#include <list>
#include <initializer_list>
#include <algorithm>

#include "globalVulkan.h"

struct VertexBuffer;
struct IndexBuffer;
struct DrawTarget;
struct Shader;
struct SwapChain;

struct DrawShader{
    Shader* shad;
    std::vector<DrawTarget*> drawTargs;//per swapchain
    std::vector<std::vector<VkCommandBuffer>*> drawCmds;

    void RegisterSwapChain(SwapChain* swap);
};
struct DrawObj{
    std::vector<VertexBuffer*> vertBuffs;
    IndexBuffer* indexBuff;
    std::vector<DrawShader*> drawShads;

    std::list<SwapChain*> registeredSwaps;

    DrawObj(std::initializer_list<VertexBuffer*> vertBuffs,IndexBuffer* index);
    DrawObj(std::initializer_list<VertexBuffer*> vertBuffs);
    void RegisterShader(Shader* shad);
    void RegisterSwapChain(SwapChain* swap);
    void Draw(Shader* shad,SwapChain* swap);
};