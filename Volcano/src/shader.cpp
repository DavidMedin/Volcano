#include "shader.h"
#include "swapchain.h"
#include "vertexbuffer.h"
std::list<SwapChain*> swapList;
std::list<Shader*> shadList;


//ReadFile("path/to/file.txt",&(char*)buff,&(size_t)buffSize);
int ReadTheFile(const char* path, char** buff, unsigned int* buffSize) {
    //init the file
    FILE* file = NULL;
    if (fopen_s(&file, path, "rb")) {
        Error("Couldn't open file! %s\n", path);
        return 0;
    }
    //get the size of the file
    fseek(file, 0L, SEEK_END);//put the cursor to the end
    *buffSize = ftell(file);//read the cursor pos
    fseek(file, 0L, SEEK_SET);//put the cursor to the front
    //allocate mem for the buffer from size
    *buff = (char*)calloc(*buffSize, 1);
    fread(*buff, 1, *buffSize, file);
    fclose(file);
    return 1;
}

Draw::~Draw() {
    vkDestroyPipeline(swapchain->device->device, graphicsPipeline, NULL);
    for (auto frame : frames) {
        vkDestroyFramebuffer(swapchain->device->device, frame, NULL);
    }
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(swapchain->device->device, available[i], NULL);
        vkDestroySemaphore(swapchain->device->device, presentable[i], NULL);
        vkDestroyFence(swapchain->device->device, fences[i], NULL);
    }
}

void CreateFramebuffers(VkDevice device, VkRenderPass render, VkImageView* imageViews, unsigned int imageCount, VkExtent2D extent, std::vector<VkFramebuffer>* framebuffIn) {
    for (unsigned int i = 0; i < imageCount; i++) {
        VkFramebuffer frame;
        VkFramebufferCreateInfo frameInfo = {};
        frameInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameInfo.renderPass = render;//framebuffer must be compatible with this render pass.neat
        frameInfo.attachmentCount = 1;
        frameInfo.pAttachments = &imageViews[i];
        frameInfo.width = extent.width;
        frameInfo.height = extent.height;
        frameInfo.layers = 1;
        if (vkCreateFramebuffer(device, &frameInfo, NULL, &frame) != VK_SUCCESS) {
            Error("    framebuffer create failed\n");
            return;
        }
        (*framebuffIn)[i] = frame;
    }
}
VkShaderModule CreateShaderModule(Device* device, char* code, unsigned int codeSize) {
    VkShaderModuleCreateInfo shaderInfo = {};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = (size_t)codeSize;
    shaderInfo.pCode = (uint32_t*)code;
    VkShaderModule shader;
    if (vkCreateShaderModule(device->device, &shaderInfo, NULL, &shader) != VK_SUCCESS) {
        Error("CreateShaderModule failed to create a shader!\n");
        return NULL;
    }
    return shader;
}

// void CreateShader(Device device,VkRenderPass renderpass, SwapChain swap,const char* vertexShader,const char* fragmentShader, Shader* shad){
Shader::Shader(Device* device, ShaderGroup* shaderGroup, const char* vertexShader, const char* fragmentShader) {
    this->device = device;
    this->group = shaderGroup;
    cmdPool = CreateCommandPool(this->device->device, &device->families);
    pipelineLayout = CreatePipeLayout(device);
    shadModCount = 2;
    shadMods = (VkShaderModule*)malloc(sizeof(VkShaderModule));

    const char* shaders[] = { vertexShader,fragmentShader };
    for (unsigned int i = 0; i < shadModCount; i++) {
        unsigned int codeSize = 0;
        char* code;
        ReadTheFile(shaders[i], &code, &codeSize);
        shadMods[i] = CreateShaderModule(device, code, codeSize);
        free(code);
    }

    shadList.push_back(this);
}

void Shader::RegisterSwapChain(SwapChain* swap) {
    Draw* command = new Draw;
    command->swapchain = swap;
    command->renderpass = CreateRenderpass(swap->GetFormat(), device, group);
    command->graphicsPipeline = CreateGraphicsPipeline(device, pipelineLayout, command->renderpass->renderpass, swap->swapExtent, this);
    command->frames.resize(swap->imageCount);
    CreateFramebuffers(device->device, command->renderpass->renderpass, swap->imageViews, swap->imageCount, swap->swapExtent, &command->frames);
    command->drawCommands = CreateCommandBuffers(device, cmdPool, swap->imageCount);
    FillCommandBuffers(swap->swapExtent, &command->frames, command->graphicsPipeline, command->renderpass->renderpass, this, command->drawCommands);

    command->imageFence.resize(swap->imageCount, VK_NULL_HANDLE);
    command->available.resize(MAX_FRAMES_IN_FLIGHT);
    command->presentable.resize(MAX_FRAMES_IN_FLIGHT);
    command->fences.resize(MAX_FRAMES_IN_FLIGHT);
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CreateSemaphore(device->device, &(command->available[i]));
        CreateSemaphore(device->device, &(command->presentable[i]));
        CreateFence(device->device, &(command->fences[i]));
    }
    commands.push_back(command);
    //I hope this works
}

void Shader::DestroySwapChain(SwapChain* swap) {
    for (auto command : commands) {
        if (command->swapchain == swap) {
            commands.remove(command);
            delete command;
            return;
        }
    }
}

void Shader::DrawFrame(SwapChain* swap) {
    static unsigned int nextFrame = 0;
    for (auto targetCommands : commands) {
        if (targetCommands->swapchain == swap) {
            vkWaitForFences(device->device, 1, &targetCommands->fences[nextFrame], VK_TRUE, UINT64_MAX);
            // vkResetFences(device->device,1,&targetCommands->fences[nextFrame]);

            unsigned int imageIndex;
            VkResult rez = vkAcquireNextImageKHR(device->device, swap->swapChain, UINT64_MAX, targetCommands->available[nextFrame], nullptr, &imageIndex);
            //if(rez == VK_ERROR_OUT_OF_DATE_KHR || rez == VK_SUBOPTIMAL_KHR|| swap->windowResized){
            //    swap->windowResized = false;
            //    swap->Recreate();
            //}else if(rez != VK_SUCCESS){
            //    Error("Failed to acquire a swapchain image\n");
            //    return;
            //}

            if (targetCommands->imageFence[imageIndex] != VK_NULL_HANDLE) {
                vkWaitForFences(device->device, 1, &targetCommands->imageFence[imageIndex], VK_TRUE, UINT64_MAX);
            }
            vkResetFences(device->device, 1, &targetCommands->fences[nextFrame]);
            targetCommands->imageFence[imageIndex] = targetCommands->fences[nextFrame];


            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &targetCommands->available[nextFrame];
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &(*targetCommands->drawCommands)[imageIndex];
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &targetCommands->presentable[nextFrame];
            if (vkQueueSubmit(device->queues[0], 1, &submitInfo, targetCommands->fences[nextFrame]) != VK_SUCCESS) {
                Error("Couldn't send command buffer to graphics queue\n");
            }

            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &targetCommands->presentable[nextFrame];
            presentInfo.swapchainCount = 1;//whaaaatttt?
            presentInfo.pSwapchains = &swap->swapChain;
            presentInfo.pImageIndices = &imageIndex;
            rez = vkQueuePresentKHR(device->queues[1], &presentInfo);
            if (rez == VK_ERROR_OUT_OF_DATE_KHR || rez == VK_SUBOPTIMAL_KHR || swap->windowResized) {
                swap->windowResized = false;
                swap->Recreate();
            }
            else if (rez != VK_SUCCESS) {
                Error("Failed to acquire a swapchain image\n");
            }
            // vkQueueWaitIdle(device->queues[1]);
            nextFrame = (nextFrame + 1) % MAX_FRAMES_IN_FLIGHT;
            return;
        }
    }
}

void Shader::RegisterVertexBuffer(VertexBuffer* buff) {
    if (vertBuffs.size() != 0) {
        if (vertNum != buff->vertexNum) {
            Error("WA-HA-HOO! WHY is the number of vertices in this vertex buffer NOT equal the vertex number of the shader!?\n");
        }
    }
    else {
        vertNum = buff->vertexNum;
    }
    vertBuffs.push_back(buff);
    buff->shaders.push_back(this);
    buff->uses++;
    //rebuild graphics pipeline with updated vertex buffer list
    for (auto command : commands) {
        vkDestroyPipeline(device->device, command->graphicsPipeline, NULL);
        command->graphicsPipeline = CreateGraphicsPipeline(device, pipelineLayout, command->renderpass->renderpass, command->swapchain->swapExtent, this);
        FillCommandBuffers(command->swapchain->swapExtent, &command->frames, command->graphicsPipeline, command->renderpass->renderpass, this, command->drawCommands);
    }
}


Shader::~Shader() {
    for (unsigned int i = 0; i < shadModCount; i++) {
        vkDestroyShaderModule(device->device, shadMods[i], NULL);
    }
    vkDestroyCommandPool(device->device, cmdPool, NULL);
    vkDestroyPipelineLayout(device->device, pipelineLayout, NULL);
    if (commands.size() != 0) {
        Error("Didn't destroy all swapchains. Please destroy them before the shader.\n");
    }
    std::list<VertexBuffer*> delList;
    for (auto buff : vertBuffs) {
        buff->uses--;
        if (buff->uses == 0) {
            delList.push_back(buff);
        }
    }
    for (auto buff : delList) {
        vertBuffs.remove(buff);
        delete buff;
    }
}

// void Shader::RecalculateSwapchain(SwapChain* swap){

// }

VkPipelineLayout CreatePipeLayout(Device* device) {
    //create the pipline layout
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //a bunch of other info goes here later

    if (vkCreatePipelineLayout(device->device, &layoutInfo, NULL, &pipelineLayout) != VK_SUCCESS) {
        Error("    PipelineLayout\n");
    }
    return pipelineLayout;
}

VkPipeline CreateGraphicsPipeline(Device* device, VkPipelineLayout layout, VkRenderPass renderPass, VkExtent2D viewExtent, Shader* shad) {
    //assign the shader stage (vertexShaderMod to vertex shader stage)
    VkPipelineShaderStageCreateInfo shaderStages[2] = { {},{} };
    VkShaderStageFlagBits bits[2] = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };
    //put into a loop for now, to add geometry shader you just need to add into the bits, shader stages, moduels and destory (needs to be more elegant)
    for (unsigned int i = 0; i < 2; i++) {
        shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[i].stage = bits[i];
        shaderStages[i].module = shad->shadMods[i];
        shaderStages[i].pName = "main";//this means we can define the entrypoint!!
        shaderStages[i].pSpecializationInfo = NULL;//defines constants so Vulkan can optimize the shader around them! Very cool.
    }
    //describe the vertex input (like bindings and attributes)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    unsigned int attribCount = 0;
    vertexInputInfo.vertexBindingDescriptionCount = (unsigned int)shad->vertBuffs.size();
    std::vector<VkVertexInputBindingDescription> tmpBindings;
    std::vector<VkVertexInputAttributeDescription> tmpAttribs;
    for (auto buff : shad->vertBuffs) {
        tmpBindings.push_back(buff->bindDesc);
        for (auto attrib : buff->attribDescs) {
            tmpAttribs.push_back(attrib);
            attribCount++;
        }
    }
    vertexInputInfo.pVertexBindingDescriptions = tmpBindings.data();
    vertexInputInfo.vertexAttributeDescriptionCount = attribCount;
    vertexInputInfo.pVertexAttributeDescriptions = tmpAttribs.data();
    //input assembly, triangles vs lines
    VkPipelineInputAssemblyStateCreateInfo inputAssInfo = {};
    inputAssInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssInfo.primitiveRestartEnable = VK_FALSE;
    //define the viewport and scissors(?)
    VkPipelineViewportStateCreateInfo viewState = {};
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)viewExtent.width;
    viewport.height = (float)viewExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    //scissors
    VkRect2D scissor = { 0 };
    scissor.extent = viewExtent;
    VkOffset2D off = { 0,0 };
    scissor.offset = off;
    viewState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewState.viewportCount = 1;
    viewState.pViewports = &viewport;
    viewState.scissorCount = 1;
    viewState.pScissors = &scissor;

    //create the rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;//fragments outide of the near/far plane are clamped
    rasterizer.rasterizerDiscardEnable = VK_FALSE;//basically disables the rasterizer (no output)
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;//fill triangles with fragments
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;//add depth offset basded on slope
    //multisampling
    VkPipelineMultisampleStateCreateInfo multisample = {};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.sampleShadingEnable = VK_FALSE;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;//other members aren't touched, if enabling, we'll need to touch
    //blendAttach is the color blending settings per-framebuffer (we have 1 framebuffer, so we only need one)
    VkPipelineColorBlendAttachmentState blendAttach = {};
    blendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttach.blendEnable = VK_TRUE;
    //my copy pasta is exquisite
    blendAttach.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttach.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttach.alphaBlendOp = VK_BLEND_OP_ADD;
    //not to define blending constants for ALL framebuffers
    VkPipelineColorBlendStateCreateInfo blendGlobal = {};
    blendGlobal.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendGlobal.logicOpEnable = VK_FALSE;//this was fucking with me thro validation layers
    blendGlobal.logicOp = VK_LOGIC_OP_COPY;//copy the blend effect to the framebuffer? (like not adding or something)
    blendGlobal.attachmentCount = 1;
    blendGlobal.pAttachments = &blendAttach;
    //might need to add something if something breaks for blendGlobal...
    //where uniforms reside

    //create the graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;//vertex and fragment I rekon
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;//what data is input into the vertex shader
    pipelineInfo.pInputAssemblyState = &inputAssInfo;//triangles or points?
    pipelineInfo.pViewportState = &viewState;//how to modify the output (output famebuffer rez and scissors)
    pipelineInfo.pRasterizationState = &rasterizer;//configure the 'render process'
    pipelineInfo.pMultisampleState = &multisample;//one pass. i.e. disabled
    pipelineInfo.pColorBlendState = &blendGlobal;//how to combine a translucent surface and a opaque surface

    pipelineInfo.layout = layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;//this is an index

    pipelineInfo.basePipelineIndex = -1;//we don't want to derive from another pipeline

    VkPipeline result;
    if (vkCreateGraphicsPipelines(device->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &result) != VK_SUCCESS) {
        Error("    Graphics Pipeline create failed\n");
    }

    return result;
}