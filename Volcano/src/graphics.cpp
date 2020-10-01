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
    *buff = (char*)calloc(*buffSize,1);
    fread(*buff,1,*buffSize,file);
    fclose(file);
    return 1;
}




VkShaderModule CreateShaderModule(Device* device,char* code,unsigned int codeSize){
    VkShaderModuleCreateInfo shaderInfo = {};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = (size_t)codeSize;
    shaderInfo.pCode = (uint32_t*)code;
    VkShaderModule shader;
    if(vkCreateShaderModule(device->device,&shaderInfo,NULL,&shader) != VK_SUCCESS){
        Error("CreateShaderModule failed to create a shader!\n");
        return NULL;
    }
    return shader;
}

// void CreateShader(Device device,VkRenderPass renderpass, SwapChain swap,const char* vertexShader,const char* fragmentShader, Shader* shad){
Shader::Shader(Device* device,unsigned int shaderGroup, SwapChain* swap,const char* vertexShader,const char* fragmentShader){
    // *shad = malloc(sizeof(struct Shader));
    // Shader deShad = *shad;
    this->device = device;
    // this->renderpass = renderpass;
    this->shaderGroup = shaderGroup;
    cmdPool = CreateCommandPool(this->device->device,&device->families);
    pipelineLayout = CreatePipeLayout(device);
    shadModCount = 2;
    shadMods = (VkShaderModule*)malloc(sizeof(VkShaderModule));

    const char* shaders[] = {vertexShader,fragmentShader};
    for(unsigned int i = 0; i < shadModCount;i++){
        unsigned int codeSize = 0;
        char* code;
        ReadTheFile(shaders[i],&code,&codeSize);
        shadMods[i] = CreateShaderModule(device,code,codeSize);
        free(code);
    }
}
void Shader::RegisterSwapChains(std::initializer_list<SwapChain*> swaps){
    for(auto swap : swaps){
        Command* tmpCmd = new Command;
        tmpCmd->swapchain = swap;
        tmpCmd->renderpass = GetRenderpass(swap->GetFormat(),device,shaderGroup);
        tmpCmd->graphicsPipeline = CreateGraphicsPipeline(device,pipelineLayout,tmpCmd->renderpass->renderpass,swap->swapExtent,this);
        tmpCmd->drawCommands = CreateCommandBuffers(device,cmdPool,swap->imageCount);

        bool found = false;
        for(auto frame : swap->frames){
            if(frame->renderpass == tmpCmd->renderpass){
                FillCommandBuffers(swap->swapExtent,&frame->framebuffers,tmpCmd->graphicsPipeline,tmpCmd->renderpass->renderpass,tmpCmd->drawCommands);
                found = true;
            }
        }
        if(found == false) Error("Wha oh! this Command's renderpass doesn't match any renderpasses in the given swapchain!\n");

        swap->shaders.push_back(this);
        tmpCmd->imageFence.resize(swap->imageCount,VK_NULL_HANDLE);
        tmpCmd->available.resize(MAX_FRAMES_IN_FLIGHT);
        tmpCmd->presentable.resize(MAX_FRAMES_IN_FLIGHT);
        tmpCmd->fences.resize(MAX_FRAMES_IN_FLIGHT);
        for(unsigned int i = 0;i < MAX_FRAMES_IN_FLIGHT;i++){
            CreateSemaphore(device->device,&(tmpCmd->available[i]));
            CreateSemaphore(device->device,&(tmpCmd->presentable[i]));
            CreateFence(device->device,&(tmpCmd->fences[i]));
        }
        commands.push_back(tmpCmd);
    }
}

void Shader::DrawFrame(SwapChain* swap){
    static unsigned int nextFrame = 0;
    for(auto targetCommands : commands){
        if(targetCommands->swapchain == swap){
            vkWaitForFences(device->device,1,&targetCommands->fences[nextFrame],VK_TRUE,UINT64_MAX);
            // vkResetFences(device->device,1,&targetCommands->fences[nextFrame]);

            unsigned int imageIndex;
            vkAcquireNextImageKHR(device->device,swap->swapChain,UINT64_MAX,targetCommands->available[nextFrame],nullptr,&imageIndex);

            if(targetCommands->imageFence[imageIndex] != VK_NULL_HANDLE){
                vkWaitForFences(device->device,1,&targetCommands->imageFence[imageIndex],VK_TRUE,UINT64_MAX);
            }
            vkResetFences(device->device,1,&targetCommands->fences[nextFrame]);
            targetCommands->imageFence[imageIndex] = targetCommands->fences[nextFrame];


            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &targetCommands->available[nextFrame];
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &(*targetCommands->drawCommands)[imageIndex];
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &targetCommands->presentable[nextFrame];
            if(vkQueueSubmit(device->queues[0],1,&submitInfo,targetCommands->fences[nextFrame]) != VK_SUCCESS){
                Error("Couldn't send command buffer to graphics queue\n");
            }

            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &targetCommands->presentable[nextFrame];
            presentInfo.swapchainCount = 1;//whaaaatttt?
            presentInfo.pSwapchains = &swap->swapChain;
            presentInfo.pImageIndices = &imageIndex;
            vkQueuePresentKHR(device->queues[1],&presentInfo);
            // vkQueueWaitIdle(device->queues[1]);
            nextFrame = (nextFrame + 1) % MAX_FRAMES_IN_FLIGHT;
            return;
        }
    }
}


Shader::~Shader(){
    for(unsigned int i = 0; i < shadModCount;i++){
        vkDestroyShaderModule(device->device,shadMods[i],NULL);
    }
    vkDestroyCommandPool(device->device,cmdPool,NULL);
	vkDestroyPipelineLayout(device->device,pipelineLayout,NULL);
    for(auto command : commands){
        vkDestroyPipeline(device->device,command->graphicsPipeline,NULL);
        for(unsigned int i = 0;i < MAX_FRAMES_IN_FLIGHT;i++){
            vkDestroySemaphore(device->device,command->available[i],NULL);
            vkDestroySemaphore(device->device,command->presentable[i],NULL);
            vkDestroyFence(device->device,command->fences[i],NULL);
        }
    }
}

void Shader::RecalculateSwapchain(SwapChain* swap){

}

VkPipelineLayout CreatePipeLayout(Device* device){
    //create the pipline layout
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //a bunch of other info goes here later

    if(vkCreatePipelineLayout(device->device,&layoutInfo,NULL,&pipelineLayout) != VK_SUCCESS){
        Error("    PipelineLayout\n");
    }
    return pipelineLayout;
}

VkPipeline CreateGraphicsPipeline(Device* device,VkPipelineLayout layout, VkRenderPass renderPass,VkExtent2D viewExtent,Shader* shad){
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
    VkPipelineShaderStageCreateInfo shaderStages[2] = {{},{}};
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
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType =  VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
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
    if(vkCreateGraphicsPipelines(device->device,VK_NULL_HANDLE,1,&pipelineInfo,NULL,&result) != VK_SUCCESS){
        Error("    Graphics Pipeline create failed\n");
    }

    // for(unsigned int i = 0;i < 2;i++){
    //     vkDestroyShaderModule(device,shaderModules[i],NULL);
    // }
    return result;
}






//device,imageCount,images,and imageFormat is input
//imageViews is an output arguments
int CreateImageViews(VkDevice device,unsigned int imageCount,VkImage* images,VkSurfaceFormatKHR* imageFormat,VkImageView** imageViews){
	if(imageCount > 0){
		*imageViews = (VkImageView*)malloc(sizeof(VkImageView)*imageCount);
		for(unsigned int i =0;i < imageCount;i++){
			//create the creation struct
			VkImageViewCreateInfo imageInfo = {};
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
    unsigned int fifo = 0;
	for(unsigned int i=0;i < dets->presentCount;i++){
		switch(dets->presentModes[i]){
            case VK_PRESENT_MODE_MAILBOX_KHR:
                return i;
            case VK_PRESENT_MODE_FIFO_KHR:
                fifo = i;
        }
        return fifo;
	}
	printf("no triple buffer present mode, falling back to double buffer\n");
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swap_GetBestSwapExtent(SwapChainSupportDetails* dets,unsigned int targWidth,unsigned int targHeight){
	if(dets->capabilities.currentExtent.width != UINT32_MAX){
		return dets->capabilities.currentExtent;
	}else{
    	VkExtent2D actualExtent = {targWidth,targHeight};
    	actualExtent.width = std::min(std::max(dets->capabilities.minImageExtent.width,actualExtent.width), dets->capabilities.maxImageExtent.width);
    	actualExtent.height = std::min(std::max(dets->capabilities.minImageExtent.height,actualExtent.height),dets->capabilities.maxImageExtent.height);
    	return actualExtent;
	}
}

void RecreateSwapchain(Device* devDets,VkSurfaceKHR surface,SwapChain* swap){
    VkDevice device = devDets->device;
	QueueFamilyIndex* famz = &devDets->families;
    // renderpassCount = 0;
    if(!IsDeviceCompatible(swap->device->phyDev,surface,swap->device->phyProps,&swap->device->families,&swap->swapDets)){
        Error("This surface doesn't comply with the picked device. wack\n");
    }
	swap->chosenFormat = Swap_GetBestSurfaceFormat(&swap->swapDets);
	swap->chosenPresent = Swap_GetBestPresentMode(&swap->swapDets);
	VkSurfaceFormatKHR* format = &(swap->swapDets.formats[swap->chosenFormat]);
	VkPresentModeKHR present = swap->swapDets.presentModes[swap->chosenPresent];
	VkExtent2D extent = Swap_GetBestSwapExtent(&swap->swapDets,WIDTH,HEIGHT);

	swap->imageCount = swap->swapDets.capabilities.minImageCount + 1;
	//if maximage is 0, unlimited images; otherwise get the min of the two
	if (swap->swapDets.capabilities.maxImageCount > 0 && swap->imageCount > swap->swapDets.capabilities.maxImageCount) {
		swap->imageCount = swap->swapDets.capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR swapInfo = {};
	swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapInfo.surface = surface;
	swapInfo.minImageCount = swap->imageCount;
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

	swapInfo.preTransform = swap->swapDets.capabilities.currentTransform; // no applied transform
	swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //no alpha blending with other windows

	swapInfo.presentMode = present;
	swapInfo.clipped = VK_TRUE;//don't care about pixels that are obstructed by other windows

	swapInfo.oldSwapchain = VK_NULL_HANDLE;//for when we need to change the swapchain (like when we need to resize the window)

	if(vkCreateSwapchainKHR(device,&swapInfo,NULL,&swap->swapChain) != VK_SUCCESS){
		Error("couldn't create a swap chain! oh no boo hoo.\n");
	}

	swap->swapExtent = extent;
}

// int CreateSwapChain(Device* devDets,VkSurfaceKHR surface, SwapChain* swapchain){
SwapChain::SwapChain(Device* devDets,VkSurfaceKHR surface){
    this->device = devDets;
	RecreateSwapchain(devDets,surface,this);
    //get the VkImages
	imageCount = 0;
	vkGetSwapchainImagesKHR(device->device,swapChain,&imageCount,NULL);
	images = (VkImage*)malloc(sizeof(VkImage)*imageCount);
	vkGetSwapchainImagesKHR(device->device,swapChain,&imageCount,images);
	if(imageCount == 0){
		Error("the swap chain has zero images!\n");
	}

	if(!CreateImageViews(device->device,imageCount,images,&swapDets.formats[chosenFormat],&imageViews)){
		Error("Couldn't create image views!\n");
	}
}
SwapChain::~SwapChain(){
    vkDestroySwapchainKHR(device->device,swapChain,NULL);
    for(auto frame : frames){
        for(auto framebuffer : frame->framebuffers){
            vkDestroyFramebuffer(device->device,framebuffer,NULL);
        }
    }
}

void CreateFramebuffers(VkDevice device, VkRenderPass render,VkImageView* imageViews,unsigned int imageCount,VkExtent2D extent, std::vector<VkFramebuffer>* framebuffIn){
    for(unsigned int i = 0;i < imageCount;i++){
        VkFramebuffer frame;
        VkFramebufferCreateInfo frameInfo ={};
		frameInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameInfo.renderPass = render;//framebuffer must be compatible with this render pass.neat
		frameInfo.attachmentCount = 1;
		frameInfo.pAttachments = &imageViews[i];
		frameInfo.width = extent.width;
		frameInfo.height = extent.height;
		frameInfo.layers = 1;
        if(vkCreateFramebuffer(device,&frameInfo,NULL,&frame) != VK_SUCCESS){
			Error("    framebuffer create failed\n");
			return;
		}
        (*framebuffIn)[i] = frame;
    }
}

void SwapChain::Recreate(){
    vkDeviceWaitIdle(device->device);
    //create new swapchain
    vkDestroySwapchainKHR(device->device,swapChain,NULL);
    //destroy image views
    for(unsigned int i = 0; i < imageCount;i++){
        vkDestroyImageView(device->device,imageViews[i],NULL);
    }
    RecreateSwapchain(device,surface,this);
    //create image views
    if(!CreateImageViews(device->device,imageCount,images,&swapDets.formats[chosenFormat],&imageViews)){
		Error("Couldn't create image views!\n");
	}
    //renderpass refresh
    for(auto frame : frames){
        GetRenderpass(this->GetFormat(),device,frame->renderpass->shaderGroup);
        //delete/create new framebuffers
        for(auto framebuffer : frame->framebuffers){
            vkDestroyFramebuffer(device->device,framebuffer,NULL);
        }
        
    }
    //need lists of like render passes in format with different shader specs
    //each shader and struct framebuffer is attached to a list of those render passes
    //when refreshing swapchain check if shader spec and format is already existing and use that rather than creating a new one
}


void SwapChain::RegisterRenderPasses(std::initializer_list<std::shared_ptr<RenderPass>> renderpasses){
    for(auto renderpass : renderpasses){
        Framebuffer* tmpFrame = new Framebuffer;
        // this->renderpasses.push_back(renderpass);
        tmpFrame->renderpass = renderpass;
        tmpFrame->framebuffers.resize(imageCount);
        // for(unsigned int i = 0; i < imageCount;i++){
        //     VkFramebuffer frame;
        //     VkFramebufferCreateInfo frameInfo ={};
		// 	frameInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		// 	frameInfo.renderPass = renderpass->renderpass;//framebuffer must be compatible with this render pass. neat
		// 	frameInfo.attachmentCount = 1;
		// 	frameInfo.pAttachments = &imageViews[i];
		// 	frameInfo.width = swapExtent.width;
		// 	frameInfo.height = swapExtent.height;
		// 	frameInfo.layers = 1;
        //     if(vkCreateFramebuffer(device->device,&frameInfo,NULL,&frame) != VK_SUCCESS){
		// 		Error("    framebuffer create failed\n");
		// 		return;
		// 	}
        //     tmpFrame->framebuffers[i] = frame;
        // }
        CreateFramebuffers(device->device,renderpass->renderpass,imageViews,imageCount,swapExtent,&tmpFrame->framebuffers);
        frames.push_back(tmpFrame);
    }
}

VkFormat SwapChain::GetFormat(){
    return swapDets.formats[chosenFormat].format;
}