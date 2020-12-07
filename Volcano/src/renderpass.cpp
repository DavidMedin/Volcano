#include "renderpass.h"
std::vector<std::list<std::weak_ptr<RenderPass>>> renderpassRegistry;

ShaderGroup::ShaderGroup(){

}

ShaderGroup::ShaderGroup(int index){
    this->index = index;
    VkAttachmentDescription* frameAttachment = (VkAttachmentDescription*)calloc(1,sizeof(VkAttachmentDescription));
    // frameAttachment.format = format;
    frameAttachment->flags = 0;
    frameAttachment->samples = VK_SAMPLE_COUNT_1_BIT;
    frameAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;//clear the framebuffer right before rendering to it (no acid effect)
    frameAttachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;//duh
    frameAttachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//not using stencils (yet)
    frameAttachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    frameAttachment->initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//don't care what layout it is in beforehand (we're clearing it anyways)
    frameAttachment->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//giving it to the swapchain afterwards so this is the best choice

    //this is going to be given to the subpass, it is like an adapter from subpass-type to color famebuffer type or whatever you are inputing/outputing
    VkAttachmentReference* frameAttachRef = (VkAttachmentReference*)calloc(1,sizeof(VkAttachmentReference));
    frameAttachRef->attachment = 0;//an index, not a count
    frameAttachRef->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//this attachment is best for color

    //create a subpass
    VkSubpassDescription* subpass = (VkSubpassDescription*)calloc(1,sizeof(VkSubpassDescription));
    subpass->flags = 0;
    subpass->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;//we are doing graphics stuff, not compute stuff
    subpass->colorAttachmentCount = 1;
    subpass->pColorAttachments = frameAttachRef;//this is index 0,
    //referenced in the fragment shader by "layout(location=0) out vec4 outColor"!

    //create a render pass
    defaultpass = (VkRenderPassCreateInfo*)calloc(1,sizeof(VkRenderPassCreateInfo));
    clearerpass = (VkRenderPassCreateInfo*)calloc(1,sizeof(VkRenderPassCreateInfo));
    defaultpass->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    defaultpass->attachmentCount = 1;
    defaultpass->pAttachments = frameAttachment;
    defaultpass->subpassCount = 1;
    defaultpass->pSubpasses = subpass;
    defaultpass->flags = 0;
    defaultpass->pNext = VK_NULL_HANDLE;

    VkSubpassDependency* deps = (VkSubpassDependency*)calloc(1,sizeof(VkSubpassDependency));
    deps->srcSubpass = VK_SUBPASS_EXTERNAL;//the initial subpass WILL happen before
    deps->dstSubpass = 0;//our specified subpass index
    deps->srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;//we are talking about the color output
    deps->srcAccessMask = 0;//says 'don't use it'?
    deps->dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;//says when the first stage is done with the color attachment
    deps->dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;//wer're writing I guess

    defaultpass->dependencyCount = 1;
    defaultpass->pDependencies = deps;//deps are like gates between subpasses that open when conditions are met

    memcpy(clearerpass,defaultpass,sizeof(VkRenderPassCreateInfo));
    VkAttachmentDescription* clearAttach = (VkAttachmentDescription*)calloc(1,sizeof(VkAttachmentDescription));
    memcpy(clearAttach,frameAttachment,sizeof(VkAttachmentDescription));
    clearAttach->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    clearAttach->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    clearerpass->pAttachments = clearAttach;
}

RenderPass::RenderPass(ShaderGroup* group){
    this->group = group;
}

void _CreateRenderPass(VkFormat format, Device* device,ShaderGroup* group,RenderPass* pass){
    //define the input/output format
    VkRenderPassCreateInfo* createQueue[2] = {group->defaultpass,group->clearerpass};
    VkRenderPass* createDst[2] = {&pass->renderpass,&pass->clearpass};
    for(unsigned int i =0;i  <2;i++){
        for(unsigned int u = 0;u < createQueue[i]->attachmentCount;u++){
            // (createQueue[i]->pAttachments->format) = format;
            memcpy((void*)(&createQueue[i]->pAttachments[u].format),&format,sizeof(VkFormat));
        }
        if(vkCreateRenderPass(device->device,createQueue[i],NULL,createDst[i]) != VK_SUCCESS){
            Error("    Renderpass failed\n");
        }
    }

}

void DeleteRenderpass(RenderPass* renderpass){
    //I hate this
    std::list<std::weak_ptr<RenderPass>>::iterator i = renderpassRegistry[renderpass->group->index].begin();
    for(auto render: renderpassRegistry[renderpass->group->index]){
        if(render.expired()){// a little dangerous but whatever
            renderpassRegistry[renderpass->group->index].erase(i);
            vkDestroyRenderPass(renderpass->device,renderpass->renderpass,NULL);
            vkDestroyRenderPass(renderpass->device,renderpass->clearpass,NULL);
            printf("Deleted renderpass\n");
            return;
        }
        i++;
    }
    Error("AAAAHAHAHHAHHAHAAH! WHY CAN'T THE RENDERPASS DELETER FIND THE RENDERPASS! AHAHHAAH!\n");
}

std::shared_ptr<RenderPass> GetRenderpass(VkFormat format,Device* device,unsigned int shaderGroup){
    //check if shaderGroup is valid, create otherwise
    if(renderpassRegistry.capacity() >= shaderGroup+1){
        //check if this renderpass exists
        for(auto renderpass : renderpassRegistry[shaderGroup]){
            if(((std::shared_ptr<RenderPass>)renderpass)->format == format){
                printf("(Get) Retrived used renderpass\n");
                return renderpass.lock();
            }
        }
    }
    Error("No renderpasses with that format!\n");
    return NULL;
}
std::shared_ptr<RenderPass> CreateRenderpass(VkFormat format,Device* device,ShaderGroup* group){
 //check if shaderGroup is valid, create otherwise
    if(renderpassRegistry.capacity() >= group->index+1){
        //check if this renderpass exists
        for(auto renderpass : renderpassRegistry[group->index]){
            if(((std::shared_ptr<RenderPass>)renderpass)->format == format){
                printf("(Create) Retrived used renderpass\n");
                return renderpass.lock();
            }
        }
    }else{
        renderpassRegistry.resize(group->index+1);
    }
    //create new renderpass,register it, and return a pointer to it
    RenderPass* tmpRender = new RenderPass(group);
    tmpRender->format = format;
    // tmpRender->renderpass = _CreateRenderPass(format,device);
    // tmpRender->group = group;
    tmpRender->device = device->device;
    _CreateRenderPass(format,device,group,tmpRender);

    std::shared_ptr<RenderPass> tmpPass = std::shared_ptr<RenderPass>(tmpRender,DeleteRenderpass);
    renderpassRegistry[group->index].push_back(std::weak_ptr<RenderPass>(tmpPass));
    printf("Createing new renderpass\n");
    return tmpPass;
}

void DestroyRenderpasses(){
    for(auto group: renderpassRegistry){
        for(auto render: group){
            vkDestroyRenderPass(((std::shared_ptr<RenderPass>)render)->device,((std::shared_ptr<RenderPass>)render)->renderpass,NULL);
            vkDestroyRenderPass(((std::shared_ptr<RenderPass>)render)->device,((std::shared_ptr<RenderPass>)render)->clearpass,NULL);
        }
    }
}