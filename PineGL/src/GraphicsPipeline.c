#include "GraphicsPipeline.h"
//ReadFile("path/to/file.txt",&(char*)buff,&(size_t)buffSize);
int ReadTheFile(const char* path,char** buff,unsigned int* buffSize){
    //init the file
    FILE* file = NULL;
    if(fopen_s(&file,path,"r")){
        printf("Couldn't open file! %s\n",path);
        return 0;
    }
    //get the size of the file
    fseek(file,0L,SEEK_END);//put the cursor to the end
    *buffSize = ftell(file);//read the cursor pos
    fseek(file,0L,SEEK_SET);//put the cursor to the front
    //allocate mem for the buffer from size
    *buff = calloc(*buffSize,1);
    fread(*buff,1,*buffSize,file);
    fclose(file);
    return 1;
}


VkShaderModule CreateShaderModule(VkDevice device,char* code,unsigned int codeSize){
    VkShaderModuleCreateInfo shaderInfo = {0};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = (size_t)codeSize;
    shaderInfo.pCode = (uint32_t*)code;

    VkShaderModule shader;
    if(vkCreateShaderModule(device,&shaderInfo,NULL,&shader) != VK_SUCCESS){
        printf("CreateShaderModule failed to create a shader!\n");
        return NULL;
    }
    return shader;
}

int CreateGraphicsPipeline(VkDevice device){
    //read the files
    char* fragmentCode;
    unsigned int fragmentSize=0;
    char* vertexCode;
    unsigned int vertexSize=0;
    if(!ReadTheFile("PineGL/src/shaders/fragment.spv",&fragmentCode,&fragmentSize)) return 0;
    if(!ReadTheFile("PineGL/src/shaders/vertex.spv",&vertexCode,&vertexSize)) return 0;
    //create the shader modules (giving vulkan the code?)
    VkShaderModule shaderModules[2] = {CreateShaderModule(device,fragmentCode,fragmentSize),CreateShaderModule(device,vertexCode,vertexSize)};
    //assign the shader stage (vertexShaderMod to vertex shader stage)
    VkPipelineShaderStageCreateInfo shaderStages[2] = {0,0};
    VkShaderStageFlagBits bits[2] = {VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT};
    //put into a loop for now, to add geometry shader you just need to add into the bits, shader stages, moduels and destory (needs to be more elegant)
    for(unsigned int i = 0;i < 2;i++){
        shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[i].stage = bits[i];
        shaderStages[i].module = shaderModules[i];
        shaderStages[i].pName = "main";//this means we can define the entrypoint!!
        shaderStages[i].pSpecializationInfo = NULL;//defines constants so Vulkan can optimize the shader around them! Very cool.
    }

    for(unsigned int i = 0;i < 2;i++){
        vkDestroyShaderModule(device,shaderModules[i],NULL);
    }
    return 1;
}