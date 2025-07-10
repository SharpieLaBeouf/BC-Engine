#include "BC_PCH.h"

#define VMA_IMPLEMENTATION

#include <vulkan/vulkan.h>
#if defined(BC_PLATFORM_WINDOWS)
#include <vma/vk_mem_alloc.h>
#elif defined(BC_PLATFORM_LINUX)
#include <vk_mem_alloc.h>
#endif