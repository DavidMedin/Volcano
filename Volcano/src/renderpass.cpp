#include "renderpass.h"
std::vector<std::list<std::weak_ptr<RenderPass>>> renderpassRegistry;
//need a way to override these things
VkRenderPass _CreateRenderPass(VkFormat format, Device* device){
    VkRenderPass tmpRender;
    //define the input/output format
    VkAttachmentDescription frameAttachment = {0};
    frameAttachment.format = format;
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
    VkRenderPassCreateInfo renderInfo = {};
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

    if(vkCreateRenderPass(device->device,&renderInfo,NULL,&tmpRender) != VK_SUCCESS){
        Error("    Renderpass failed\n");
    }
    return tmpRender;
}

void DeleteRenderpass(RenderPass* renderpass){
    //I hate this
    std::list<std::weak_ptr<RenderPass>>::iterator i = renderpassRegistry[renderpass->group->index].begin();
    for(auto render: renderpassRegistry[renderpass->group->index]){
        if(render.expired()){// a little dangerous but whatever
            renderpassRegistry[renderpass->group->index].erase(i);
            vkDestroyRenderPass(renderpass->device,renderpass->renderpass,NULL);
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
    RenderPass* tmpRender = new RenderPass;
    tmpRender->format = format;
    tmpRender->renderpass = _CreateRenderPass(format,device);
    tmpRender->group = group;
    tmpRender->device = device->device;

    std::shared_ptr<RenderPass> tmpPass = std::shared_ptr<RenderPass>(tmpRender,DeleteRenderpass);
    renderpassRegistry[group->index].push_back(std::weak_ptr<RenderPass>(tmpPass));
    printf("Createing new renderpass\n");
    return tmpPass;
}

void DestroyRenderpasses(){
    for(auto group: renderpassRegistry){
        for(auto render: group){
            vkDestroyRenderPass(((std::shared_ptr<RenderPass>)render)->device,((std::shared_ptr<RenderPass>)render)->renderpass,NULL);
        }
    }
}