#include "graphics.h"
//ReadFile("path/to/file.txt",&(char*)buff,&(size_t)buffSize);
int ReadTheFile(const char* path,char** buff,unsigned int* buffSize){
    //init the file
    FILE* file = NULL;
    if(fopen_s(&file,path,"rb")){
        Error("Couldn't open file! %s\n",path);
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




VkShaderModule CreateShaderModule(Device device,char* code,unsigned int codeSize){
    VkShaderModuleCreateInfo shaderInfo = {0};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = (size_t)codeSize;
    shaderInfo.pCode = (uint32_t*)code;
    VkShaderModule shader;
    if(vkCreateShaderModule(device.device,&shaderInfo,NULL,&shader) != VK_SUCCESS){
        Error("CreateShaderModule failed to create a shader!\n");
        return NULL;
    }
    return shader;
}

void CreateShader(Device device,VkRenderPass renderpass, SwapChain swap,const char* vertexShader,const char* fragmentShader, Shader* shad){
    *shad = malloc(sizeof(struct Shader));
    Shader deShad = *shad;

    deShad->shadModCount = 2;
    deShad->shadMods = malloc(sizeof(VkShaderModule));

    const char* shaders[] = {vertexShader,fragmentShader};
    for(unsigned int i = 0; i < deShad->shadModCount;i++){
        unsigned int codeSize = 0;
        char* code;
        ReadTheFile(shaders[i],&code,&codeSize);
        deShad->shadMods[i] = CreateShaderModule(device,code,codeSize);
        free(code);
    }
    CreateGraphicsPipeline(device,renderpass,swap->swapExtent,deShad);
}
// void CreateShader(Device device,VkRenderPass renderpass, SwapChain* swap, VkShaderModule vertexShader,VkShaderModule fragmentShader, Shader* shad){
//      *shad = malloc(sizeof(struct Shader));
//     Shader deShad = *shad;

//     deShad->shadModCount = 2;
//     deShad->shadMods = malloc(sizeof(VkShaderModule));
//     deShad->shadMods[0] = vertexShader;
//     deShad->shadMods[1] = fragmentShader;

//     CreateGraphicsPipeline(device,renderpass,swap->swapExtent,deShad);
// }

void DestroyShader(Device device, Shader shad){
    for(unsigned int i = 0; i < shad->shadModCount;i++){
        vkDestroyShaderModule(device.device,shad->shadMods[i],NULL);
    }
    vkDestroyPipeline(device.device,shad->graphicsPipeline,NULL);
	vkDestroyPipelineLayout(device.device,shad->pipelineLayout,NULL);
}

void CreateGraphicsPipeline(Device device,VkRenderPass renderPass,VkExtent2D viewExtent,Shader shad){
    //read the files
    // char* fragmentCode;
    // unsigned int fragmentSize=0;
    // char* vertexCode;
    // unsigned int vertexSize=0;
    // //PineGL/src/shaders/fragment.spv
    // //PineGL/src/shaders/vertex.spv
    // if(!ReadTheFile("Volcano/src/shaders/fragment.spv",&fragmentCode,&fragmentSize)) return 0;
    // if(!ReadTheFile("Volcano/src/shaders/vertex.spv",&vertexCode,&vertexSize)) return 0;
    // //create the shader modules (giving vulkan the code?)
    // VkShaderModule shaderModules[2] = {CreateShaderModule(device,fragmentCode,fragmentSize),CreateShaderModule(device,vertexCode,vertexSize)};
    // if(shaderModules[0] == NULL || shaderModules[1] == NULL){
    //     Error("one of the shader modules are bad\n");
    // }
    //assign the shader stage (vertexShaderMod to vertex shader stage)
    VkPipelineShaderStageCreateInfo shaderStages[2] = {0,0};
    VkShaderStageFlagBits bits[2] = {VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT};
    //put into a loop for now, to add geometry shader you just need to add into the bits, shader stages, moduels and destory (needs to be more elegant)
    for(unsigned int i = 0;i < 2;i++){
        shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[i].stage = bits[i];
        shaderStages[i].module = shad->shadMods[i];
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
    //where uniforms reside
    //create the pipline layout
    VkPipelineLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //a bunch of other info goes here later

    if(vkCreatePipelineLayout(device.device,&layoutInfo,NULL,&shad->pipelineLayout) != VK_SUCCESS){
        Error("    PipelineLayout\n");
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

    pipelineInfo.layout = shad->pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;//this is an index

    pipelineInfo.basePipelineIndex = -1;//we don't want to derive from another pipeline

    if(vkCreateGraphicsPipelines(device.device,VK_NULL_HANDLE,1,&pipelineInfo,NULL,&shad->graphicsPipeline) != VK_SUCCESS){
        Error("    Graphics Pipeline create failed\n");
    }

    // for(unsigned int i = 0;i < 2;i++){
    //     vkDestroyShaderModule(device,shaderModules[i],NULL);
    // }
}






//device,imageCount,images,and imageFormat is input
//imageViews is an output arguments
int CreateImageViews(VkDevice device,unsigned int imageCount,VkImage* images,VkSurfaceFormatKHR* imageFormat,VkImageView** imageViews){
	if(imageCount > 0){
		*imageViews = malloc(sizeof(VkImageView*)*imageCount);
		for(unsigned int i =0;i < imageCount;i++){
			//create the creation struct
			VkImageViewCreateInfo imageInfo = {0};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageInfo.image = images[i];
			//we are dealing with 2d things here
			imageInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageInfo.format = imageFormat->format;

			imageInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			//describes image access and use
			imageInfo.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;//useed for color?
			imageInfo.subresourceRange.baseMipLevel = 0;
			imageInfo.subresourceRange.levelCount = 1;
			imageInfo.subresourceRange.baseArrayLayer = 0;
			imageInfo.subresourceRange.layerCount = 1;
			if(vkCreateImageView(device,&imageInfo,NULL,&(*imageViews)[i])!= VK_SUCCESS){
				Error("Couldn't create a imageView!\n");
				return 0;
			}
		}
	}else{
		Error("CreateImageViews was given 0 as an imageCount! that is wrong.\n");
		return 0;
	}
	return 1;
}

//returns the index into formats that should be used
unsigned int Swap_GetBestSurfaceFormat(SwapChainSupportDetails* dets){
	for(unsigned int i=0;i < dets->formatCount;i++){
		if(dets->formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && dets->formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return i;
		}
	}
	Error("the ideal colorspace wasn't provided in formats!\n");
	return 0;
}

unsigned int Swap_GetBestPresentMode(SwapChainSupportDetails* dets){
	for(unsigned int i=0;i < dets->presentCount;i++){
		if(dets->presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR){
			return i;
		}
	}
	Error("no triple buffer present mode, falling back to double buffer");
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swap_GetBestSwapExtent(SwapChainSupportDetails* dets){
	if(dets->capabilities.currentExtent.width != UINT32_MAX){
		return dets->capabilities.currentExtent;
	}else{
    	VkExtent2D actualExtent = {WIDTH,HEIGHT};
    	actualExtent.width = min(max(dets->capabilities.minImageExtent.width,actualExtent.width), dets->capabilities.maxImageExtent.width);
    	actualExtent.height = min(max(dets->capabilities.minImageExtent.height,actualExtent.height),dets->capabilities.maxImageExtent.height);
    	return actualExtent;
	}
}

int CreateSwapChain(Device* devDets,VkSurfaceKHR surface, SwapChain* swapchain){
	*swapchain = malloc(sizeof(struct SwapChain));
	//shortcuts
	SwapChain deSwap = *swapchain;
	VkDevice device = devDets->device;
	QueueFamilyIndex* famz = &devDets->families;
    (*swapchain)->renderpassCount = 0;

	deSwap->swapDets = &devDets->swapSupport;
	deSwap->chosenFormat = Swap_GetBestSurfaceFormat(deSwap->swapDets);
	deSwap->chosenPresent = Swap_GetBestPresentMode(deSwap->swapDets);
	VkSurfaceFormatKHR* format = &(deSwap->swapDets->formats[deSwap->chosenFormat]);
	VkPresentModeKHR present = deSwap->swapDets->presentModes[deSwap->chosenPresent];
	VkExtent2D extent = Swap_GetBestSwapExtent(deSwap->swapDets);

	unsigned int imageCount = deSwap->swapDets->capabilities.minImageCount + 1;
	//if maximage is 0, unlimited images; otherwise get the min of the two
	if (deSwap->swapDets->capabilities.maxImageCount > 0 && imageCount > deSwap->swapDets->capabilities.maxImageCount) {
		imageCount = deSwap->swapDets->capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR swapInfo = { 0 };
	swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapInfo.surface = surface;
	swapInfo.minImageCount = imageCount;
	swapInfo.imageFormat = format->format;
	swapInfo.imageColorSpace = format->colorSpace;
	swapInfo.imageExtent = extent;
	swapInfo.imageArrayLayers = 1;
	swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	//taking the availability of .graphics and .presentation for granted
		if(famz->presentation == famz->graphics){
		swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//only one family can push to the swapchain
		swapInfo.queueFamilyIndexCount = 0;
		swapInfo.pQueueFamilyIndices = NULL;
	}else{
		swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//more than one can push, need to juggle permissions tho
		swapInfo.queueFamilyIndexCount = 2;
		unsigned int indices[] = {famz->graphics,famz->presentation};
		swapInfo.pQueueFamilyIndices = indices;
	}

	swapInfo.preTransform = deSwap->swapDets->capabilities.currentTransform; // no applied transform
	swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //no alpha blending with other windows

	swapInfo.presentMode = present;
	swapInfo.clipped = VK_TRUE;//don't care about pixels that are obstructed by other windows

	swapInfo.oldSwapchain = VK_NULL_HANDLE;//for when we need to change the swapchain (like when we need to resize the window)

	if(vkCreateSwapchainKHR(device,&swapInfo,NULL,&deSwap->swapChain) != VK_SUCCESS){
		Error("couldn't create a swap chain! oh no boo hoo.\n");
		return 0;
	}
	//get the VkImages
	deSwap->imageCount = 0;
	vkGetSwapchainImagesKHR(device,deSwap->swapChain,&deSwap->imageCount,NULL);
	deSwap->images = malloc(sizeof(VkImage)*deSwap->imageCount);
	vkGetSwapchainImagesKHR(device,deSwap->swapChain,&deSwap->imageCount,deSwap->images);
	if(deSwap->imageCount == 0){
		Error("the swap chain has zero images!\n");
	}

	if(!CreateImageViews(device,deSwap->imageCount,deSwap->images,&deSwap->swapDets->formats[deSwap->chosenFormat],&deSwap->imageViews)){
		Error("Couldn't create image views!\n");
		return 0;
	}
	deSwap->swapExtent = extent;
	return 1;
}
