#include "source.h"
/*
notes for future david:

window struct shouldn't contain a device, but a swapchain from a device.
each swapchain should be initialized with a shader, and the swapchain should contain pointers to arrays of shader-draw-commandbuffers/shaders (maybe a struct?).
also init swapchain in createwindow pleeeeas
*/



Instance instance;
Device device;
Window window;

Shader shad;

// VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;//we'll do mutiple of these in the future
// VkDevice device = VK_NULL_HANDLE;
// //queues
// VkQueue graphicsQueue = VK_NULL_HANDLE;
// VkQueue presentQueue = VK_NULL_HANDLE;
// //swapchain
// VkSwapchainKHR swap = VK_NULL_HANDLE;


// VkPipelineLayout pipelineLayout;
VkRenderPass renderPass;
// VkPipeline graphicsPipeline;

VkFramebuffer* framebuffers;
unsigned int framebufferCount;//should equal DeviceDetails.swapChain->imageCount

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
	vkDeviceWaitIdle(device.device);//finish everything it is doing so the semaphores are no longer in use

	for(unsigned int i = 0;i < semaphoreCount;i++){
		vkDestroySemaphore(device.device,imageDrawReady[i],NULL);
		vkDestroySemaphore(device.device,imagePrezReady[i],NULL);
		vkDestroyFence(device.device,inFlightFences[i],NULL);
	}

	vkDestroyCommandPool(device.device,commandPool,NULL);

	for(unsigned int i = 0;i < framebufferCount;i++){
		vkDestroyFramebuffer(device.device,framebuffers[i],NULL);
	}


	vkDestroyRenderPass(device.device,renderPass,NULL);


	DestroyShader(device,shad);
	DestoryWindow(instance,device,window);


	 DestoryInstance(instance);

	DestroyGLFW();

}


void DrawFrame(){
	vkWaitForFences(device.device,1,&inFlightFences[nextFrame],VK_TRUE,UINT64_MAX);
	//get an avaiable image from the swap chain
	unsigned int imageIndex;
	vkAcquireNextImageKHR(device.device,window->swapchain->swapChain,UINT64_MAX,imageDrawReady[nextFrame],VK_NULL_HANDLE,&imageIndex);

	vkResetFences(device.device,1,&inFlightFences[nextFrame]);
	//makes sure we arn't drawing to the same image
	if(swapImageFence[imageIndex] != VK_NULL_HANDLE){
		//the swapchain is being drawn too. wait
		vkWaitForFences(device.device,1,&swapImageFence[imageIndex],VK_TRUE,UINT64_MAX);
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
	if(vkQueueSubmit(device.queues[0],1,&submitInfo,inFlightFences[nextFrame]) != VK_SUCCESS){
		printf("couldn't send command buffer to graphics queue\n");
		return;
	}
	//give the finished image to the swap chain for prezzy
	VkPresentInfoKHR presentInfo = {0};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &imagePrezReady[nextFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &window->swapchain->swapChain;
	presentInfo.pImageIndices = &imageIndex;
	vkQueuePresentKHR(device.queues[1],&presentInfo);
	vkDeviceWaitIdle(device.device);//do we need this?

	nextFrame = (nextFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

/*thinking about lists
	for(something* element = GetIter(list);Iter(list,element);element++){


	}

*/

//btw vulkan types and defines are way too fucking long
int main() {
	CreateWindow("TestWindow",&instance,&device,&window);
	

	CreateRenderPass(&window,1,&device,&renderPass);
	

	// //create framebuffers
	// if(!CreateFramebuffers(device.device,renderPass,window,&framebuffers,&framebufferCount)){
	// 	printf("Couldn't create Framebuffer!\n");
	// }

	CreateShader(device,renderPass,window->swapchain,"Volcano/src/shaders/vertex.spv","Volcano/src/shaders/fragment.spv",&shad);
	//create a command pool (stores the command buffers more efficently)
	if(!CreateCommandPool(device.device,&(device.families),&commandPool)){
		printf("Couldn't create commandPool\n");
	}
	//allocate mem for command buffers
	if(!CreateCommandBuffers(device.device,commandPool,framebufferCount,&commandBuffers)){
		printf("Couldn't allocate memory for command buffers!\n");
	}
	//fill the command buffers
	commandBufferCount = framebufferCount;
	if(!FillCommandBuffers(window->swapchain->swapExtent,framebuffers,framebufferCount,shad->graphicsPipeline,renderPass,commandBufferCount,commandBuffers)){
		printf("Couldn't fill the command buffers\n");
	}

	VkSemaphore** semass[] = {&imageDrawReady,&imagePrezReady};
	for(unsigned int u = 0;u < 2;u++){
		*(semass[u]) = malloc(sizeof(VkFence*)*semaphoreCount);
		for(unsigned int i = 0;i < semaphoreCount;i++){
			if(!CreateSemaphore(device.device,&((*semass[u])[i]))){
				printf("Could't create a semaphore! : %d : %d \n",i,u);
			}
		}
	}
	inFlightFences = malloc(sizeof(VkFence*) * semaphoreCount);
	for(unsigned int i = 0;i < semaphoreCount;i++){
		if(!CreateFence(device.device,&inFlightFences[i])){
			printf("Couldn't create a fence! : %d\n",i);
		}
	}
	swapImageFence = malloc(sizeof(VkFence*)*window->swapchain->imageCount);
	for(unsigned int i = 0;i < window->swapchain->imageCount;i++){
		swapImageFence[i] = VK_NULL_HANDLE;
	}

	//game loop
	while (!glfwWindowShouldClose(window->window)) {
		glfwPollEvents();
		DrawFrame();
	}
	Shutdown();
	return 0;
}
