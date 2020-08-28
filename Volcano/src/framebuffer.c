#include "framebuffer.h"

int CreateFramebuffers(VkDevice device,VkRenderPass* renderpasses,unsigned int renderCount,Window win){
    //init the memory where the framebuffers lie
    unsigned int viewCount = win->swapchain->imageCount;
	//allocate and write to renderers
	win->swapchain->renderpasses = malloc(sizeof(VkRenderPass)*renderCount);
	memcpy(win->swapchain->renderpasses,renderpasses,sizeof(VkRenderPass)*renderCount);
    
	win->swapchain->framebuffers = malloc(sizeof(VkFramebuffer)*renderCount);
	VkFramebuffer** framebuffs = win->swapchain->framebuffers;
	VkFramebuffer* ptr2 = malloc(sizeof(VkFramebuffer)*viewCount*renderCount);
	
	//write to part 1 with part 2
	for(unsigned int i = 0;i < renderCount;i++){
		framebuffs[i] = ptr2[i*viewCount];
	}
	
	for(unsigned int r = 0;r < renderCount;r++){
		for(unsigned int i = 0;i < viewCount;i++){
			VkFramebufferCreateInfo frameInfo ={0};
			frameInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameInfo.renderPass = renderpasses[r];//framebuffer must be compatible with this render pass. neat
			frameInfo.attachmentCount = 1;
			frameInfo.pAttachments = &win->swapchain->imageViews[i];
			frameInfo.width = win->swapchain->swapExtent.width;
			frameInfo.height = win->swapchain->swapExtent.height;
			frameInfo.layers = 1;

			if(vkCreateFramebuffer(device,&frameInfo,NULL,&framebuffs[r][i]) != VK_SUCCESS){
				Error("    framebuffer create failed\n");
				return 0;
			}
		}
	}
    return 1;
}