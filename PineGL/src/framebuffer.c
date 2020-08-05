#include "framebuffer.h"

int CreateFramebuffers(VkDevice device,VkRenderPass renderPass,VkExtent2D swapExtent,VkImageView* views,unsigned int viewCount,VkFramebuffer** framebuffers,unsigned int* framebufferCount){
    //init the memory where the framebuffers lie
    *framebufferCount = viewCount;
    *framebuffers = malloc(sizeof(VkFramebuffer)**framebufferCount);
    
    for(unsigned int i = 0;i < viewCount;i++){
        VkFramebufferCreateInfo frameInfo ={0};
        frameInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameInfo.renderPass = renderPass;//framebuffer must be compatible with this render pass. neat
        frameInfo.attachmentCount = 1;
        frameInfo.pAttachments = &views[i];
        frameInfo.width = swapExtent.width;
        frameInfo.height = swapExtent.height;
        frameInfo.layers = 1;

        if(vkCreateFramebuffer(device,&frameInfo,NULL,&((*framebuffers)[i])) != VK_SUCCESS){
            printf("    framebuffer create failed\n");
            return 0;
        }
    }
    return 1;
}