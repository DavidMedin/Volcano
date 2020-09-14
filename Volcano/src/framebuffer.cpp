#include "framebuffer.h"

void CreateFramebuffers(VkDevice device,VkRenderPass* renderpasses,unsigned int renderCount,Window* win){
    // //init the memory where the framebuffers lie
    // unsigned int viewCount = win->swapchain->imageCount;
	// SwapChain* swap = win->swapchain;
	// //allocate and write to renderers
	// unsigned int finalRenderCount = swap->renderpassCount + renderCount;
	// VkRenderPass* tmpRenderers = (VkRenderPass*)malloc(sizeof(VkRenderPass)*finalRenderCount);
	// if(swap->renderpassCount) memcpy(tmpRenderers,swap->renderpasses,swap->renderpassCount*sizeof(VkRenderPass));
	// memcpy(tmpRenderers+(swap->renderpassCount*sizeof(VkRenderPass)),renderpasses,sizeof(VkRenderPass)*renderCount);
    // swap->renderpasses = tmpRenderers;

	// VkFramebuffer** ptr1 = (VkFramebuffer**)malloc(sizeof(VkFramebuffer)*finalRenderCount);
	// VkFramebuffer* ptr2 = (VkFramebuffer*)malloc(sizeof(VkFramebuffer)*viewCount*finalRenderCount);
	// if(swap->renderpassCount) (ptr2,swap->framebuffers[0],swap->renderpassCount*sizeof(VkFramebuffer));
	// swap->framebuffers = ptr1;
	// //write to part 1 with part 2
	// for(unsigned int i = 0;i < finalRenderCount;i++){
	// 	ptr1[i] = &ptr2[i*viewCount];
	// }
	
	// for(unsigned int r = swap->renderpassCount;r < finalRenderCount;r++){
	// 	for(unsigned int i = 0;i < viewCount;i++){
	// 		VkFramebufferCreateInfo frameInfo ={};
	// 		frameInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	// 		frameInfo.renderPass = swap->renderpasses[r];//framebuffer must be compatible with this render pass. neat
	// 		frameInfo.attachmentCount = 1;
	// 		frameInfo.pAttachments = &swap->imageViews[i];
	// 		frameInfo.width = swap->swapExtent.width;
	// 		frameInfo.height = swap->swapExtent.height;
	// 		frameInfo.layers = 1;

	// 		if(vkCreateFramebuffer(device,&frameInfo,NULL,&ptr1[r][i]) != VK_SUCCESS){
	// 			Error("    framebuffer create failed\n");
	// 			return;
	// 		}
	// 	}
	// }
	// swap->renderpassCount = finalRenderCount;

}