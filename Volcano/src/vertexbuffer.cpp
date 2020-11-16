#include "vertexbuffer.h"
#include "shader.h"

void CopyBuffer(Device* device,VkBuffer src,VkBuffer dst,VkDeviceSize size){
	VkCommandBufferAllocateInfo cmdAllocInfo = {};
	cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdAllocInfo.commandPool = device->transferCmdPool;
	cmdAllocInfo.commandBufferCount = 1;

	VkCommandBuffer cmdBuff;
	vkAllocateCommandBuffers(device->device,&cmdAllocInfo,&cmdBuff);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmdBuff,&beginInfo);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(cmdBuff,src,dst,1,&copyRegion);

	vkEndCommandBuffer(cmdBuff);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuff;
	vkQueueSubmit(device->queues[0],1,&submitInfo,VK_NULL_HANDLE);
	vkQueueWaitIdle(device->queues[0]);
	vkFreeCommandBuffers(device->device,device->transferCmdPool,1,&cmdBuff);
}

VertexBuffer::~VertexBuffer(){
	for(auto shad : shaders){
		//should remove all refs from shaders
		shad->vertBuffs.remove(this);
	}
	vkDestroyBuffer(device->device,stageBuff,NULL);
	vkDestroyBuffer(device->device,fastBuff,NULL);
	vkFreeMemory(device->device,fastBuffMem,NULL);
	vkFreeMemory(device->device,stageMem,NULL);
}

void VertexBuffer::MapData(void** data){
	vkMapMemory(device->device,stageMem,0,memSize,0,data);
}
void VertexBuffer::UnMapData(){
	vkUnmapMemory(device->device,stageMem);
	CopyBuffer(device,stageBuff,fastBuff,memSize);
}
void CreateBuffer(Device* device,VkDeviceSize size, int usage,VkSharingMode share,VkMemoryPropertyFlags props,VkBuffer* buff,VkDeviceMemory* buffMem){
	VkBufferCreateInfo buffInfo = {};
	buffInfo.flags=0;
	buffInfo.pNext=0;
	buffInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffInfo.size = size;
	buffInfo.usage = usage;
	buffInfo.sharingMode = share;//cause only for graphics queue
	if(vkCreateBuffer(device->device,&buffInfo,NULL,buff)!= VK_SUCCESS){
		Error("Couldn't create a vertex buffer!\n");
	}

	VkMemoryRequirements memRequire;
	vkGetBufferMemoryRequirements(device->device,*buff,&memRequire);
	int typeIndex = -1;
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(device->phyDev,&memProps);
	for(int i = 0; i < (int)memProps.memoryTypeCount;i++){
		if(memRequire.memoryTypeBits & (1 << i) && (memProps.memoryTypes[i].propertyFlags & props)==props){
			typeIndex = i;
			break;
		}
	}
	if(typeIndex == -1) {Error("No memory types work for this vertex buffer. Or you're just stupid\n");}

	//allocate memory on the GPU for the buffer
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.pNext = 0;
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequire.size;
	allocInfo.memoryTypeIndex = (unsigned int)typeIndex;

	if(vkAllocateMemory(device->device,&allocInfo,NULL,buffMem) != VK_SUCCESS){
		Error("Couldn't allocate memory for the vertex buffer!\n");
	}

	vkBindBufferMemory(device->device,*buff,*buffMem,0);
}