
#include "shader.h"
#include "swapchain.h"
#include "vertexbuffer.h"
#include "window.h"

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

DrawTarget::~DrawTarget() {
    vkDestroyPipeline(swapchain->device->device, graphicsPipeline, NULL);
    for (auto frame : frames) {
        vkDestroyFramebuffer(swapchain->device->device, frame, NULL);
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
VkShaderModule CreateShaderModule(Device* device, std::vector<uint32_t>* code) {
    VkShaderModuleCreateInfo shaderInfo = {};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = (size_t)code->size()*4; //*4 for the convertion from uint32_t size to bytes
    shaderInfo.pCode = (uint32_t*)code->data();
    VkShaderModule shader;
    if (vkCreateShaderModule(device->device, &shaderInfo, NULL, &shader) != VK_SUCCESS) {
        Error("CreateShaderModule failed to create a shader!\n");
        return NULL;
    }
    return shader;
}

Shader::Shader(ShaderGroup* shaderGroup, const char* glslShader){
    std::string vertexPath = std::string(glslShader);
    std::string fragmentPath = std::string(glslShader);
    vertexPath.append(".vert");
    fragmentPath.append(".frag");

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    this->device = GetCurrentDevice();
    this->group = shaderGroup;
    cmdPool = CreateCommandPool(this->device);
    pipelineLayout = CreatePipeLayout(device);
    shadModCount = 2;
    shadMods = new ShaderMod[2];

    const char* shaders[] = { vertexPath.c_str(),fragmentPath.c_str() };
    const shaderc_shader_kind shad_progress[2] = {shaderc_vertex_shader,shaderc_fragment_shader};
    for (unsigned int i = 0; i < shadModCount; i++) {
        unsigned int codeSize = 0;
        char* glslCode;
        ReadTheFile(shaders[i], &glslCode, &codeSize);

        shaderc::SpvCompilationResult compileRez = compiler.CompileGlslToSpv(glslCode,codeSize,shad_progress[i],glslShader);
        if(compileRez.GetCompilationStatus() != shaderc_compilation_status_success){
            Error("%s\n",compileRez.GetErrorMessage().c_str());
        }
        shadMods[i].code = std::vector<uint32_t>(compileRez.cbegin(),compileRez.cend());
        shadMods[i].mod = CreateShaderModule(device, &shadMods[i].code);
        free(glslCode);
    }
    //use spriv-reflect to get shader input ID of vertex Shader
    
    SpvReflectResult rez = spvReflectCreateShaderModule(shadMods[0].code.size()*4,shadMods[0].code.data(),&mod);
    assert(rez == SPV_REFLECT_RESULT_SUCCESS);

    rez = spvReflectEnumerateInputVariables(&mod,&inputCount,NULL);
    assert(rez == SPV_REFLECT_RESULT_SUCCESS);
    assert(inputCount != 0);
    inputVars = (SpvReflectInterfaceVariable**)malloc(sizeof(SpvReflectInterfaceVariable)*inputCount);
    rez = spvReflectEnumerateInputVariables(&mod,&inputCount,inputVars);
    assert(rez == SPV_REFLECT_RESULT_SUCCESS);
    assert(inputCount != 0);

    inputDescs.push_back(new ID(0,0,NULL,BufferRate::PER_VERTEX,this));

    shadList.push_back(this);
}

Shader::Shader(std::initializer_list<ID*> ids,ShaderGroup* shaderGroup,const char* vertexShader,const char* fragmentShader) {
    this->device = GetCurrentDevice();
    this->group = shaderGroup;
    cmdPool = CreateCommandPool(this->device);
    pipelineLayout = CreatePipeLayout(device);
    shadModCount = 2;
    shadMods = (ShaderMod*)malloc(sizeof(ShaderMod)*shadModCount);

    const char* shaders[] = { vertexShader,fragmentShader };
    for (unsigned int i = 0; i < shadModCount; i++) {
        unsigned int codeSize = 0;
        char* code;
        ReadTheFile(shaders[i], &code, &codeSize);
        shadMods[i].mod = CreateShaderModule(device, code, codeSize);
        free(code);
    }
    //use spriv-reflect to get shader input ID


    for(auto id : ids){
        inputDescs.push_back(id);
    }

    shadList.push_back(this);
}

void Shader::RegisterSwapChain(SwapChain* swap) {
    DrawTarget* target = new DrawTarget;
    target->swapchain = swap;
    target->renderpass = CreateRenderpass(swap->GetFormat(), device, group);
    target->graphicsPipeline = CreateGraphicsPipeline(device, pipelineLayout, target->renderpass->renderpass, swap->swapExtent, this);
    target->frames.resize(swap->imageCount);
    CreateFramebuffers(device->device, target->renderpass->renderpass, swap->imageViews, swap->imageCount, swap->swapExtent, &target->frames);


    drawTargs.push_back(target);
    //I hope this works
}

void Shader::DestroySwapChain(SwapChain* swap) {
    for (auto command : drawTargs) {
        if (command->swapchain == swap) {
            drawTargs.remove(command);
            delete command;
            return;
        }
    }
}
bool Shader::ContainsSwap(SwapChain* swap){
    for(auto targ : drawTargs){
        if(targ->swapchain == swap){
            return true;
        }
    }
    return false;
}

ID* Shader::GetNthID(unsigned int n){
    unsigned int i = 0;
    for(auto id: inputDescs){
        if(i == n) return id;
        i++;
    }
    Error("n (%u) is out of range in getNTHID\n",n);
    return nullptr;
}

Shader::~Shader() {
    for (unsigned int i = 0; i < shadModCount; i++) {
        vkDestroyShaderModule(device->device, shadMods[i].mod, NULL);
    }
    vkDestroyCommandPool(device->device, cmdPool, NULL);
    vkDestroyPipelineLayout(device->device, pipelineLayout, NULL);
    if (drawTargs.size() != 0) {
        Error("Didn't destroy all swapchains. Please destroy them before the shader.\n");
    }
    std::list<VertexBuffer*> delList;

}

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
        shaderStages[i].module = shad->shadMods[i].mod;
        shaderStages[i].pName = "main";//this means we can define the entrypoint!!
        shaderStages[i].pSpecializationInfo = NULL;//defines constants so Vulkan can optimize the shader around them! Very cool.
    }
    //describe the vertex input (like bindings and attributes)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    unsigned int attribCount = 0;
    vertexInputInfo.vertexBindingDescriptionCount = (unsigned int)shad->inputDescs.size();
    std::vector<VkVertexInputBindingDescription> tmpBindings;
    std::vector<VkVertexInputAttributeDescription> tmpAttribs;
    for (auto desc : shad->inputDescs) {
        tmpBindings.push_back(desc->bindDesc);
        for (auto attrib : desc->attribDescs) {
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
// void Shader::RegisterID(ID* id){
//     inputDescs.push_back(id);
// }
// void Shader::BakeIDs(){
//     for(auto targ : drawTargs){
//         vkDestroyPipeline(device->device,targ->graphicsPipeline,NULL);
//         targ->graphicsPipeline = CreateGraphicsPipeline(device,pipelineLayout,targ->renderpass->renderpass,targ->swapchain->swapExtent,this);
//     }
// }