#include "vertexbuffer.h"
#include "shader.h"

std::list<Buffer*> bufferList;

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


void CreateBuffer(Device* device,uint64_t size, int usage,VkSharingMode share,VkMemoryPropertyFlags props,VkBuffer* buff,VkDeviceMemory* buffMem){
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
//usage example: VK_BUFFER_USAGE_INDEXBUFFER_BIT
Buffer::Buffer(Device* device,unsigned int vertexNum, uint64_t size, int usage){
	this->device = device;
	memSize = size;
	this->vertexNum = vertexNum;
	CreateBuffer(device,size,usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_SHARING_MODE_EXCLUSIVE,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,&stageBuff,&stageMem);
	CreateBuffer(device,size,usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,VK_SHARING_MODE_EXCLUSIVE,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,&fastBuff,&fastMem);
	bufferList.push_back(this);
}
Buffer::~Buffer(){
	vkDestroyBuffer(device->device,stageBuff,NULL);
	vkDestroyBuffer(device->device,fastBuff,NULL);
	vkFreeMemory(device->device,fastMem,NULL);
	vkFreeMemory(device->device,stageMem,NULL);
}
void Buffer::MapData(void** data){
	vkMapMemory(device->device,stageMem,0,memSize,0,data);
}
void Buffer::UnMapData(){
	vkUnmapMemory(device->device,stageMem);
	CopyBuffer(device,stageBuff,fastBuff,memSize);
}
void IndexBuffer::MapData(void** data){
	indexBuff->MapData(data);
}
void IndexBuffer::UnMapData(){
	indexBuff->UnMapData();
}
unsigned int FormatSize(VkFormat format){
	switch(format){
		case VK_FORMAT_R8_SINT: return 1;
		case VK_FORMAT_R8_UINT: return 1;
		case VK_FORMAT_R8G8_SINT: return 2;
		case VK_FORMAT_R8G8_UINT: return 2;
		case VK_FORMAT_R8G8B8_SINT: return 3;
		case VK_FORMAT_R8G8B8_UINT: return 3;
		case VK_FORMAT_R16_SINT: return 2;
		case VK_FORMAT_R16_UINT: return 2;
		case VK_FORMAT_R16_SFLOAT: return 2;
		case VK_FORMAT_R16G16_SINT: return 4;
		case VK_FORMAT_R16G16_UINT: return 4;
		case VK_FORMAT_R16G16_SFLOAT: return 4;
		case VK_FORMAT_R16G16B16_SINT: return 6;
		case VK_FORMAT_R16G16B16_UINT: return 6;
		case VK_FORMAT_R16G16B16_SFLOAT: return 6;
		case VK_FORMAT_R16G16B16A16_SINT: return 8;
		case VK_FORMAT_R16G16B16A16_UINT: return 8;
		case VK_FORMAT_R16G16B16A16_SFLOAT: return 8;
		case VK_FORMAT_R32_SINT: return 4;
		case VK_FORMAT_R32_UINT: return 4;
		case VK_FORMAT_R32_SFLOAT: return 4;
		case VK_FORMAT_R32G32_SINT: return 8;
		case VK_FORMAT_R32G32_UINT: return 8;
		case VK_FORMAT_R32G32_SFLOAT: return 8;
		case VK_FORMAT_R32G32B32_SINT: return 12;
		case VK_FORMAT_R32G32B32_UINT: return 12;
		case VK_FORMAT_R32G32B32_SFLOAT: return 12;
		case VK_FORMAT_R32G32B32A32_SINT: return 16;
		case VK_FORMAT_R32G32B32A32_UINT: return 16;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
		case VK_FORMAT_R64_SINT: return 8;
		case VK_FORMAT_R64_UINT: return 8;
		case VK_FORMAT_R64_SFLOAT: return 8;
		case VK_FORMAT_R64G64_SINT: return 16;
		case VK_FORMAT_R64G64_UINT: return 16;
		case VK_FORMAT_R64G64_SFLOAT: return 16;
		case VK_FORMAT_R64G64B64_SINT: return 24;
		case VK_FORMAT_R64G64B64_UINT: return 24;
		case VK_FORMAT_R64G64B64_SFLOAT: return 24;
		case VK_FORMAT_R64G64B64A64_SINT: return 32;
		case VK_FORMAT_R64G64B64A64_UINT: return 32;
		case VK_FORMAT_R64G64B64A64_SFLOAT: return 32;
		default: Error("%d is not a valid arg to FormatSize!\n");
		return NULL;
	}
}

ID::ID(unsigned int bufferLoc,unsigned int beginLoc,unsigned int endLoc,BufferRate rate,Shader* shad){
	bindDesc = {0};
	bindDesc.binding = bufferLoc;
	bindDesc.stride = 0;//going to update this later

	if(beginLoc > endLoc) {Error("beginLoc is greater then endLoc in ID initializer!\n");}
	if(beginLoc < 0) {Error("beginLoc is less than zero! (can't be negative)\n");}
	if(endLoc > shad->inputVars.size()-1) {Error("endLoc is greater than the number of input variables! (maybe set endLoc to NULL for \'to end\')");}
	for(unsigned int i = beginLoc;i < endLoc+1;i++){
		VkVertexInputAttributeDescription desc= {0};
		desc.binding = bufferLoc;
		desc.location = i;
		desc.format = (VkFormat)shad->inputVars[i]->format;
		desc.offset = bindDesc.stride;
		bindDesc.stride += FormatSize(desc.format);
		attribDescs.push_back(desc);
	}
	size = bindDesc.stride;
}

