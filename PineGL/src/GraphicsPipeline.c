#include "GraphicsPipeline.h"
//ReadFile("path/to/file.txt",&(char*)buff,&(size_t)buffSize);
int ReadTheFile(const char* path,char** buff,unsigned int* buffSize){
    //init the file
    FILE* file = NULL;
    if(fopen_s(&file,path,"rb")){
        printf("Couldn't open file! %s\n",path);
        return 0;
    }
    //get the size of the file
    fseek(file,0L,SEEK_END);//put the cursor to the end
    *buffSize = ftell(file);//read the cursor pos
    fseek(file,0L,SEEK_SET);//put the cursor to the front
    //allocate mem for the buffer from size
    *buff = calloc(*buffSize,1);
    fread(*buff,1,*buffSize,file);
    fclose(file);
    return 1;
}

int CreateRenderPass(VkDevice device,VkSurfaceFormatKHR* format,VkRenderPass* renderPass){
    //define the input/output format
    VkAttachmentDescription frameAttachment = {0};
    frameAttachment.format = format->format;
    frameAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    frameAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//clear the framebuffer right before rendering to it (no acid effect)
    frameAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;//duh
    frameAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//not using stencils (yet)
    frameAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    frameAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//don't care what layout it is in beforehand (we're clearing it anyways)
    frameAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//giving it to the swapchain afterwards so this is the best choice

    //this is going to be given to the subpass, it is like an adapter from subpass-type to color famebuffer type or whatever you are inputing/outputing
    VkAttachmentReference frameAttachRef = {0};
    frameAttachRef.attachment = 0;//an index, not a count
    frameAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//this attachment is best for color

    //create a subpass
    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;//we are doing graphics stuff, not compute stuff
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &frameAttachRef;//this is index 0,
    //referenced in the fragment shader by "layout(location=0) out vec4 outColor"!

    //create a render pass
    VkRenderPassCreateInfo renderInfo = {0};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderInfo.attachmentCount = 1;
    renderInfo.pAttachments = &frameAttachment;
    renderInfo.subpassCount = 1;
    renderInfo.pSubpasses = &subpass;

    VkSubpassDependency deps = {0};
    deps.srcSubpass = VK_SUBPASS_EXTERNAL;//the initial subpass WILL happen before
    deps.dstSubpass = 0;//our specified subpass index
    deps.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;//we are talking about the color output
    deps.srcAccessMask = 0;//says 'don't use it'?
    deps.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;//says when the first stage is done with the color attachment
    deps.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;//wer're writing I guess

    renderInfo.dependencyCount = 1;
    renderInfo.pDependencies = &deps;//deps are like gates between subpasses that open when conditions are met

    if(vkCreateRenderPass(device,&renderInfo,NULL,renderPass) != VK_SUCCESS){
        printf("    Renderpass failed\n");
        return 0;
    }

    return 1;
}


VkShaderModule CreateShaderModule(VkDevice device,char* code,unsigned int codeSize){
    VkShaderModuleCreateInfo shaderInfo = {0};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = (size_t)codeSize;
    shaderInfo.pCode = (uint32_t*)code;
    VkShaderModule shader;
    if(vkCreateShaderModule(device,&shaderInfo,NULL,&shader) != VK_SUCCESS){
        printf("CreateShaderModule failed to create a shader!\n");
        return NULL;
    }
    return shader;
}



int CreateGraphicsPipeline(VkDevice device,VkRenderPass renderPass,VkExtent2D viewExtent,VkPipelineLayout* pipelineLayout,VkPipeline* graphicsPipeline){
    //read the files
    char* fragmentCode;
    unsigned int fragmentSize=0;
    char* vertexCode;
    unsigned int vertexSize=0;
    //PineGL/src/shaders/fragment.spv
    //PineGL/src/shaders/vertex.spv
    if(!ReadTheFile("PineGL/src/shaders/fragment.spv",&fragmentCode,&fragmentSize)) return 0;
    if(!ReadTheFile("PineGL/src/shaders/vertex.spv",&vertexCode,&vertexSize)) return 0;
    //create the shader modules (giving vulkan the code?)
    VkShaderModule shaderModules[2] = {CreateShaderModule(device,fragmentCode,fragmentSize),CreateShaderModule(device,vertexCode,vertexSize)};
    if(shaderModules[0] == NULL || shaderModules[1] == NULL){
        printf("one of the shader modules are bad\n");
    }
    //assign the shader stage (vertexShaderMod to vertex shader stage)
    VkPipelineShaderStageCreateInfo shaderStages[2] = {0,0};
    VkShaderStageFlagBits bits[2] = {VK_SHADER_STAGE_FRAGMENT_BIT,VK_SHADER_STAGE_VERTEX_BIT};
    //put into a loop for now, to add geometry shader you just need to add into the bits, shader stages, moduels and destory (needs to be more elegant)
    for(unsigned int i = 0;i < 2;i++){
        shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[i].stage = bits[i];
        shaderStages[i].module = shaderModules[i];
        shaderStages[i].pName = "main";//this means we can define the entrypoint!!
        shaderStages[i].pSpecializationInfo = NULL;//defines constants so Vulkan can optimize the shader around them! Very cool.
    }
    //describe the vertex input (like bindings and attributes)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
    vertexInputInfo.sType =  VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    //input assembly, triangles vs lines
    VkPipelineInputAssemblyStateCreateInfo inputAssInfo = {0};
    inputAssInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssInfo.primitiveRestartEnable = VK_FALSE;
    //define the viewport and scissors(?)
    VkPipelineViewportStateCreateInfo viewState = {0};
    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)viewExtent.width;
    viewport.height = (float)viewExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    //scissors
    VkRect2D scissor = {0};
    scissor.extent = viewExtent;
    VkOffset2D off = {0,0};
    scissor.offset = off;
    viewState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewState.viewportCount = 1;
    viewState.pViewports = &viewport;
    viewState.scissorCount = 1;
    viewState.pScissors = &scissor;

    //create the rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
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
    VkPipelineMultisampleStateCreateInfo multisample = {0};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.sampleShadingEnable = VK_FALSE;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;//other members aren't touched, if enabling, we'll need to touch
    //blendAttach is the color blending settings per-framebuffer (we have 1 framebuffer, so we only need one)
    VkPipelineColorBlendAttachmentState blendAttach = {0};
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
    VkPipelineColorBlendStateCreateInfo blendGlobal = {0};
    blendGlobal.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendGlobal.logicOpEnable = VK_FALSE;//this was fucking with me thro validation layers
    blendGlobal.logicOp = VK_LOGIC_OP_COPY;//copy the blend effect to the framebuffer? (like not adding or something)
    blendGlobal.attachmentCount = 1;
    blendGlobal.pAttachments = &blendAttach;
    //might need to add something if something breaks for blendGlobal...

    //create the pipline layout
    VkPipelineLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //a bunch of other info goes here later

    if(vkCreatePipelineLayout(device,&layoutInfo,NULL,pipelineLayout) != VK_SUCCESS){
        printf("    PipelineLayout\n");
        return 0;
    }

    //create the graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;//vertex and fragment I rekon
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;//what data is input into the vertex shader
    pipelineInfo.pInputAssemblyState = &inputAssInfo;//triangles or points?
    pipelineInfo.pViewportState = &viewState;//how to modify the output (output famebuffer rez and scissors)
    pipelineInfo.pRasterizationState = &rasterizer;//configure the 'render process'
    pipelineInfo.pMultisampleState = &multisample;//one pass. i.e. disabled
    pipelineInfo.pColorBlendState = &blendGlobal;//how to combine a translucent surface and a opaque surface

    pipelineInfo.layout = *pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;//this is an index

    pipelineInfo.basePipelineIndex = -1;//we don't want to derive from another pipeline

    if(vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1,&pipelineInfo,NULL,graphicsPipeline) != VK_SUCCESS){
        printf("    Graphics Pipeline create failed\n");
        return 0;
    }

    for(unsigned int i = 0;i < 2;i++){
        vkDestroyShaderModule(device,shaderModules[i],NULL);
    }
    return 1;
}