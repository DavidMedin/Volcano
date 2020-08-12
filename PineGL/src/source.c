#include "source.h"

Window window;
Instance instance;

/*
typedef struct device{
	VkCommandPool graphicsPool
}device;

typedef struct swapchain{
	jasdlkfjlksdajflkads
	unsigned int chainlinkCount;
};
		 /- image - draw command-\
swapchain - image - draw command  - shader
		\- image - draw command -/
*/
typedef struct Shader;
typedef struct Swapchain_Shader_Link{
	VkSwapchainKHR swapchain;
	unsigned int swapchainCount;
	VkCommandBuffer* drawCommands;
	Shader* shader;
}Swapchain_Shader_Link;

typedef struct Shader{
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	Swapchain_Shader_Link* drawLink;
}Shader;

typedef struct Renderer{
	VkRenderPass renderPass;
}Renderer;

// VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;//we'll do mutiple of these in the future
// VkDevice device = VK_NULL_HANDLE;
// //queues
// VkQueue graphicsQueue = VK_NULL_HANDLE;
// VkQueue presentQueue = VK_NULL_HANDLE;
// //swapchain
// VkSwapchainKHR swap = VK_NULL_HANDLE;


VkPipelineLayout pipelineLayout;
VkRenderPass renderPass;
VkPipeline graphicsPipeline;

VkFramebuffer* framebuffers;
unsigned int framebufferCount;//should equal DeviceDetails.swapChain.imageCount

VkCommandPool commandPool;//used to optimize the memory of the command buffers
VkCommandBuffer* commandBuffers;//one for each framebuffer
unsigned int commandBufferCount;//should equal framebufferCount

//used to organize the drawing and presenting of a single frame
VkSemaphore* imageDrawReady;//semaphores are for GPU-GPU sync
VkSemaphore* imagePrezReady;
VkFence* inFlightFences;//fences are like semaphores but can be queryed
VkFence* swapImageFence;//same count as swapchain images, will contain a valid pointer if it is in use (and contains the pointer to a valid vkfence(one of the 2 inflightfences))
unsigned int semaphoreCount = MAX_FRAMES_IN_FLIGHT;
unsigned int nextFrame = 0;



void Shutdown() {
	for(unsigned int i = 0;i < semaphoreCount;i++){
		vkDestroySemaphore(window->devDets.device,imageDrawReady[i],NULL);
		vkDestroySemaphore(window->devDets.device,imagePrezReady[i],NULL);
		vkDestroyFence(window->devDets.device,inFlightFences[i],NULL);
	}

	vkDestroyCommandPool(window->devDets.device,commandPool,NULL);

	for(unsigned int i = 0;i < framebufferCount;i++){
		vkDestroyFramebuffer(window->devDets.device,framebuffers[i],NULL);
	}

	vkDestroyPipeline(window->devDets.device,graphicsPipeline,NULL);
	vkDestroyPipelineLayout(window->devDets.device,pipelineLayout,NULL);
	vkDestroyRenderPass(window->devDets.device,renderPass,NULL);



	 DestoryWindow(instance,window);
	//for(unsigned int i = 0; i < window->devDets.swapChain.imageCount;i++){
	//	vkDestroyImageView(window->devDets.device,window->devDets.swapChain.imageViews[i],NULL);
	//}
	//vkDestroySwapchainKHR(window->devDets.device,window->devDets.swapChain.swapChain,NULL);


	//vkDestroyDevice(window->devDets.device, NULL);
	//vkDestroySurfaceKHR(instance->instance, window->surface, NULL);

	 DestoryInstance(instance);
	//if (enableValidationLayers) {
	//	DestroyDebugMessenger(instance->instance, instance->debugMessenger, NULL);
	//}
	//vkDestroyInstance(instance->instance, NULL);
	//glfwDestroyWindow(window->window);
	DestroyGLFW();

}


void DrawFrame(){
	vkWaitForFences(window->devDets.device,1,&inFlightFences[nextFrame],VK_TRUE,UINT64_MAX);
	//get an avaiable image from the swap chain
	unsigned int imageIndex;
	vkAcquireNextImageKHR(window->devDets.device,window->devDets.swapChain.swapChain,UINT64_MAX,imageDrawReady[nextFrame],VK_NULL_HANDLE,&imageIndex);

	vkResetFences(window->devDets.device,1,&inFlightFences[nextFrame]);
	//makes sure we arn't drawing to the same image
	if(swapImageFence[imageIndex] != VK_NULL_HANDLE){
		//the swapchain is being drawn too. wait
		vkWaitForFences(window->devDets.device,1,&swapImageFence[imageIndex],VK_TRUE,UINT64_MAX);
	}//we good to go now
	swapImageFence[imageIndex] = inFlightFences[nextFrame];

	//execute a command buffer to draw (with the said image as an arg)
	VkSubmitInfo submitInfo = {0};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageDrawReady[nextFrame];//only means anything when used when dependancies use it. I think.
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &imagePrezReady[nextFrame];
	if(vkQueueSubmit(window->devDets.queues[0],1,&submitInfo,inFlightFences[nextFrame]) != VK_SUCCESS){
		printf("couldn't send command buffer to graphics queue\n");
		return;
	}
	//give the finished image to the swap chain for prezzy
	VkPresentInfoKHR presentInfo = {0};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &imagePrezReady[nextFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &window->devDets.swapChain.swapChain;
	presentInfo.pImageIndices = &imageIndex;
	vkQueuePresentKHR(window->devDets.queues[1],&presentInfo);
	vkDeviceWaitIdle(window->devDets.device);//do we need this?

	nextFrame = (nextFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}




//btw vulkan types and defines are way too fucking long
int main() {
	//init glfw
	// if (!InitGLFW(&window)) {
	// 	printf("Panic! Glfw didn't initialize!\n");
	// }
	//create vulkan instance
	CreateWindow("TestWindow",&instance,&window);


	//find the best device (GPU in this case) and return a virtual device, physical device, and the queue families available  for that device
	if (!CreateDevices(instance->instance,window->surface,&(window->devDets))) {
		printf("Couldn't create devices!\n");
	}
	//left this out of create devices to make it more dynamic
	//create the queues for the device (graphics calls and presentation calls)
	vkGetDeviceQueue(window->devDets.device, window->devDets.families.graphics, 0, &(window->devDets.queues[0]));//3rd argument is the 0th queue of the queue family
	vkGetDeviceQueue(window->devDets.device, window->devDets.families.presentation, 0, &(window->devDets.queues[1]));

	if(!CreateRenderPass(window->devDets.device,&window->devDets.swapChain.swapDets.formats[window->devDets.swapChain.chosenFormat],&renderPass)){
		printf("couldn't create render pass\n");
	}
	if(!CreateGraphicsPipeline(window->devDets.device,renderPass,window->devDets.swapChain.swapExtent,&pipelineLayout,&graphicsPipeline)){
		printf("Couldn't create graphics pipeline");
	}

	//create framebuffers
	if(!CreateFramebuffers(window->devDets.device,renderPass,window->devDets.swapChain.swapExtent,window->devDets.swapChain.imageViews,window->devDets.swapChain.imageCount,&framebuffers,&framebufferCount)){
		printf("Couldn't create Framebuffer!\n");
	}

	//create a command pool (stores the command buffers more efficently)
	if(!CreateCommandPool(window->devDets.device,&(window->devDets.families),&commandPool)){
		printf("Couldn't create commandPool\n");
	}
	//allocate mem for command buffers
	if(!CreateCommandBuffers(window->devDets.device,commandPool,framebufferCount,&commandBuffers)){
		printf("Couldn't allocate memory for command buffers!\n");
	}
	//fill the command buffers
	commandBufferCount = framebufferCount;
	if(!FillCommandBuffers(window->devDets.swapChain.swapExtent,framebuffers,framebufferCount,graphicsPipeline,renderPass,commandBufferCount,commandBuffers)){
		printf("Couldn't fill the command buffers\n");
	}

	VkSemaphore** semass[] = {&imageDrawReady,&imagePrezReady};
	for(unsigned int u = 0;u < 2;u++){
		*(semass[u]) = malloc(sizeof(VkFence*)*semaphoreCount);
		for(unsigned int i = 0;i < semaphoreCount;i++){
			if(!CreateSemaphore(window->devDets.device,&((*semass[u])[i]))){
				printf("Could't create a semaphore! : %d : %d \n",i,u);
			}
		}
	}
	inFlightFences = malloc(sizeof(VkFence*) * semaphoreCount);
	for(unsigned int i = 0;i < semaphoreCount;i++){
		if(!CreateFence(window->devDets.device,&inFlightFences[i])){
			printf("Couldn't create a fence! : %d\n",i);
		}
	}
	swapImageFence = malloc(sizeof(VkFence*)*window->devDets.swapChain.imageCount);
	for(unsigned int i = 0;i < window->devDets.swapChain.imageCount;i++){
		swapImageFence[i] = VK_NULL_HANDLE;
	}

	//game loop
	while (!glfwWindowShouldClose(window->window)) {
		glfwPollEvents();
		DrawFrame();
	}
	vkDeviceWaitIdle(window->devDets.device);//finish everything it is doing so the semaphores are no longer in use
	Shutdown();
	return 0;
}
