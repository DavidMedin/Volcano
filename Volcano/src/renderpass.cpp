#include "renderpass.h"
#include "graphics.h"
std::vector<std::list<RenderPass*>> renderpassRegistry;
//need a way to override these things
VkRenderPass CreateRenderPass(VkFormat format, Device* device){
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
    renderpassRegistry[renderpass->shaderGroup].remove(renderpass);
    vkDestroyRenderPass(renderpass->device,renderpass->renderpass,NULL);
    printf("Deleted renderpass\n");
}

std::shared_ptr<RenderPass> GetRenderpass(VkFormat format,Device* device,unsigned int shaderGroup){
    //check if shaderGroup is valid, create otherwise
    if(renderpassRegistry.capacity() >= shaderGroup+1){
        //check if this renderpass exists
        for(auto renderpass : renderpassRegistry[shaderGroup]){
            if(renderpass->format == format){
                printf("Retrived used renderpass\n");
                return std::make_shared<RenderPass>(*renderpass);
            }
        }
    }else{
        renderpassRegistry.resize(shaderGroup+1);
    }
    //create new renderpass,register it, and return a pointer to it
    RenderPass* tmpRender = new RenderPass;
    tmpRender->format = format;
    tmpRender->renderpass = CreateRenderPass(format,device);
    tmpRender->shaderGroup = shaderGroup;
    tmpRender->device = device->device;

    renderpassRegistry[shaderGroup].push_back(tmpRender);
    printf("Createing new renderpass\n");
    return std::shared_ptr<RenderPass>(tmpRender,DeleteRenderpass);
}

void DestroyRenderpasses(){
    for(auto group: renderpassRegistry){
        for(auto render: group){
            vkDestroyRenderPass(render->device,render->renderpass,NULL);
        }
    }
}

// void CreateRenderPass(Window** win,unsigned int windowCount, Device* device,VkRenderPass* renderPass){
//     _CreateRenderPass(win[0],device,renderPass);//win is needed for formats, and all windows should have the same format
//     //create framebuffers
//     for(unsigned int i = 0;i < windowCount;i++){
//         //does nothing for now
//         CreateFramebuffers((*device).device,renderPass,1,win[i]);
//     }
// }
//for the future
// void CreateRenderPasses(Window* win,unsigned int windowCount, Device* device,VkRenderPass* renderPass){

// }