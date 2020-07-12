#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define WIDTH 800
#define HEIGHT 800

#ifdef NDEBUG
#define enableValidationLayers 0
#else
#define enableValidationLayers 1
#endif

