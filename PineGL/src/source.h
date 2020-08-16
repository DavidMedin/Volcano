#pragma once
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
#include "window.h"
#include "errorCatch.h"
//our globals --- declared in globalVulkan.h

//maximum number of frames being calclated on the GPU at one time (all on different stages)
#define MAX_FRAMES_IN_FLIGHT 2

