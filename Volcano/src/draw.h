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
struct Window;
struct Device;


struct DrawCmdGroup{
    std::vector<VkCommandBuffer>* mainDraw;
    std::vector<VkCommandBuffer>* clearDraw;
};

struct DrawObj{
    Device* device;
    std::vector<VertexBuffer*> vertBuffs;
    IndexBuffer* indexBuff;
    Shader* shad;
    unsigned int vertNum;

    std::vector<SwapChain*> registeredSwaps;
    std::vector<DrawTarget*> drawTargs;//per swapchain
    std::vector<DrawCmdGroup*> drawCmds;

    DrawObj(std::initializer_list<VertexBuffer*> vertBuffs,IndexBuffer* index,Shader* shad);
    DrawObj(std::initializer_list<VertexBuffer*> vertBuffs,Shader* shad);
    void RegisterSwapChain(SwapChain* swap);
    void QueueDraw(Window* win);
};