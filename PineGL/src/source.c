#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

//our stuff
#include "globalVulkan.h"
#include "validationLayers.h"
#include "devices.h"
#include "instance.h"
#include "GraphicsPipeline.h"
#include "framebuffer.h"
#include "commandpool.h"
//our globals --- declared in globalVulkan.h

//maximum number of frames being calclated on the GPU at one time (all on different stages)
#define MAX_FRAMES_IN_FLIGHT 2




GLFWwindow* window;
VkInstance instance;
VkDebugUtilsMessengerEXT debugMessenger;//this contains the 'instance' of the debug messenger

// VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;//we'll do mutiple of these in the future
// VkDevice device = VK_NULL_HANDLE;
// //queues
// VkQueue graphicsQueue = VK_NULL_HANDLE;
// VkQueue presentQueue = VK_NULL_HANDLE;
// //swapchain
// VkSwapchainKHR swap = VK_NULL_HANDLE;

DeviceDetails devDets;
VkSurfaceKHR surface;

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
		vkDestroySemaphore(devDets.device,imageDrawReady[i],NULL);
		vkDestroySemaphore(devDets.device,imagePrezReady[i],NULL);
		vkDestroyFence(devDets.device,inFlightFences[i],NULL);
	}

	vkDestroyCommandPool(devDets.device,commandPool,NULL);

	for(unsigned int i = 0;i < framebufferCount;i++){
		vkDestroyFramebuffer(devDets.device,framebuffers[i],NULL);
	}
	
	vkDestroyPipeline(devDets.device,graphicsPipeline,NULL);
	vkDestroyPipelineLayout(devDets.device,pipelineLayout,NULL);
	vkDestroyRenderPass(devDets.device,renderPass,NULL);

	for(unsigned int i = 0; i < devDets.swapChain.imageCount;i++){
		vkDestroyImageView(devDets.device,devDets.swapChain.imageViews[i],NULL);
	}
	vkDestroySwapchainKHR(devDets.device,devDets.swapChain.swapChain,NULL);
	

	vkDestroyDevice(devDets.device, NULL);
	vkDestroySurfaceKHR(instance, surface, NULL);
	
	
	if (enableValidationLayers) {
		DestroyDebugMessenger(instance, debugMessenger, NULL);
	}
	vkDestroyInstance(instance, NULL);
	glfwDestroyWindow(window);
	glfwTerminate();

}


void DrawFrame(){
	vkWaitForFences(devDets.device,1,&inFlightFences[nextFrame],VK_TRUE,UINT64_MAX);
	//get an avaiable image from the swap chain
	unsigned int imageIndex;
	vkAcquireNextImageKHR(devDets.device,devDets.swapChain.swapChain,UINT64_MAX,imageDrawReady[nextFrame],VK_NULL_HANDLE,&imageIndex);

	vkResetFences(devDets.device,1,&inFlightFences[nextFrame]);
	//makes sure we arn't drawing to the same image
	if(swapImageFence[imageIndex] != VK_NULL_HANDLE){
		//the swapchain is being drawn too. wait
		vkWaitForFences(devDets.device,1,&swapImageFence[imageIndex],VK_TRUE,UINT64_MAX);
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
	if(vkQueueSubmit(devDets.queues[0],1,&submitInfo,inFlightFences[nextFrame]) != VK_SUCCESS){
		printf("couldn't send command buffer to graphics queue\n");
		return;
	}
	//give the finished image to the swap chain for prezzy
	VkPresentInfoKHR presentInfo = {0};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &imagePrezReady[nextFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &devDets.swapChain.swapChain;
	presentInfo.pImageIndices = &imageIndex;
	vkQueuePresentKHR(devDets.queues[1],&presentInfo);
	vkDeviceWaitIdle(devDets.device);
	
	nextFrame = (nextFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}




//btw vulkan types and defines are way too fucking long
int main() {

	//init glfw
	if (!InitGLFW(&window)) {
		printf("Panic! Glfw didn't initialize!\n");
	}

	//create vulkan instance
	InstantiateInstance(&instance,&debugMessenger);

	//get surface from windows
	if (glfwCreateWindowSurface(instance, window, NULL, &surface)!=VK_SUCCESS) {
		printf("Couldn't create a surface!\n");
	}

	//find the best device (GPU in this case) and return a virtual device, physical device, and the queue families available  for that device
	if (!CreateDevices(instance,surface,&devDets)) {
		printf("Couldn't create devices!\n");
	}
	//left this out of create devices to make it more dynamic
	//create the queues for the device (graphics calls and presentation calls)
	vkGetDeviceQueue(devDets.device, devDets.families.graphics, 0, &(devDets.queues[0]));//3rd argument is the 0th queue of the queue family
	vkGetDeviceQueue(devDets.device, devDets.families.presentation, 0, &(devDets.queues[1]));

	if(!CreateRenderPass(devDets.device,&devDets.swapChain.swapDets.formats[devDets.swapChain.chosenFormat],&renderPass)){
		printf("couldn't create render pass\n");
	}
	if(!CreateGraphicsPipeline(devDets.device,renderPass,devDets.swapChain.swapExtent,&pipelineLayout,&graphicsPipeline)){
		printf("Couldn't create graphics pipeline");
	}

	//create framebuffers
	if(!CreateFramebuffers(devDets.device,renderPass,devDets.swapChain.swapExtent,devDets.swapChain.imageViews,devDets.swapChain.imageCount,&framebuffers,&framebufferCount)){
		printf("Couldn't create Framebuffer!\n");
	}

	//create a command pool (stores the command buffers more efficently)
	if(!CreateCommandPool(devDets.device,&(devDets.families),&commandPool)){
		printf("Couldn't create commandPool\n");
	}
	//allocate mem for command buffers
	if(!CreateCommandBuffers(devDets.device,commandPool,framebufferCount,&commandBuffers)){
		printf("Couldn't allocate memory for command buffers!\n");
	}
	//fill the command buffers
	commandBufferCount = framebufferCount;
	if(!FillCommandBuffers(devDets.swapChain.swapExtent,framebuffers,framebufferCount,graphicsPipeline,renderPass,commandBufferCount,commandBuffers)){
		printf("Couldn't fill the command buffers\n");
	}

	VkSemaphore** semass[] = {&imageDrawReady,&imagePrezReady};
	for(unsigned int u = 0;u < 2;u++){
		*(semass[u]) = malloc(sizeof(VkFence*)*semaphoreCount);
		for(unsigned int i = 0;i < semaphoreCount;i++){
			if(!CreateSemaphore(devDets.device,&((*semass[u])[i]))){
				printf("Could't create a semaphore! : %d : %d \n",i,u);
			}
		}
	}
	inFlightFences = malloc(sizeof(VkFence*) * semaphoreCount);
	for(unsigned int i = 0;i < semaphoreCount;i++){
		if(!CreateFence(devDets.device,&inFlightFences[i])){
			printf("Couldn't create a fence! : %d\n",i);
		}
	}
	swapImageFence = malloc(sizeof(VkFence*)*devDets.swapChain.imageCount);
	for(unsigned int i = 0;i < devDets.swapChain.imageCount;i++){
		swapImageFence[i] = VK_NULL_HANDLE;
	}

	//game loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		DrawFrame();
	}
	vkDeviceWaitIdle(devDets.device);//finish everything it is doing so the semaphores are no longer in use
	Shutdown();
	return 0;
}
