#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "framework/Common.h"
#include "framework/FileSystem.h"
#include "renderer/tr_local.h"
#include "idlib/math/Vector.h"
#include "idlib/math/Matrix.h"

//#include <shader-slang/slang.h>
//#include <shader-slang/slang-com-ptr.h>


#include <array>
#include <vector>

#define VMA_IMPLEMENTATION
#define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment) SDL_aligned_alloc((alignment), (size))
#define VMA_SYSTEM_ALIGNED_FREE(ptr) SDL_aligned_free((ptr))
#include <libs/vma/vk_mem_alloc.h>

#define WINDOW_TITLE "WeirdTech"

constexpr uint32_t maxFramesInFlight{ 2 };

struct Vertex {
	idVec2 pos;
	idVec3 color;
};

struct ShaderData {
	idMat4 model;
	idMat4 view;
	idMat4 proj;
};

struct ShaderDataBuffer {
	VmaAllocation allocation{ VK_NULL_HANDLE };
	VmaAllocationInfo allocationInfo{};
	VkBuffer buffer{ VK_NULL_HANDLE };
	VkDeviceAddress deviceAddress{};
};

VkInstance instance{ VK_NULL_HANDLE };
VkDevice device{ VK_NULL_HANDLE };
VkQueue queue{ VK_NULL_HANDLE };
VkSurfaceKHR surface{ VK_NULL_HANDLE };
VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
VkCommandPool commandPool{ VK_NULL_HANDLE };

VkImage depthImage;
VkImageView depthImageView;

std::vector<VkImage> swapchainImages;
std::vector<VkImageView> swapchainImageViews;

VmaAllocation vBufferAllocation{ VK_NULL_HANDLE };
VkBuffer vBuffer{ VK_NULL_HANDLE };

VmaAllocator allocator{ VK_NULL_HANDLE };
VmaAllocation depthImageAllocation;

std::array<ShaderDataBuffer, maxFramesInFlight> shaderDataBuffers;
std::array<VkCommandBuffer, maxFramesInFlight> commandBuffers;

std::array<VkFence, maxFramesInFlight> fences;
std::array<VkSemaphore, maxFramesInFlight> imageAcquiredSemaphores;
std::vector<VkSemaphore> renderCompleteSemaphores;

VkDescriptorSetLayout descriptorSetLayoutTex{ VK_NULL_HANDLE };
VkPipeline pipeline{ VK_NULL_HANDLE };
VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };

#define NUM_VERTICES 4
std::vector<Vertex> vertices = {
    {.pos=idVec2(-0.5f, -0.5f), .color=idVec3(1.0f, 0.0f, 0.0f)},
    {.pos=idVec2(0.5f, -0.5f),  .color=idVec3(0.0f, 1.0f, 0.0f)},
    {.pos=idVec2(0.5f, 0.5f),   .color=idVec3(0.0f, 0.0f, 1.0f)},
    {.pos=idVec2(-0.5f, 0.5f),  .color=idVec3(1.0f, 1.0f, 1.0f)}
};

#define NUM_VERTEX_INDICES 6
std::vector<uint16_t> vertex_indices = {
	0, 1, 2, 2, 3, 0
};

// Validation layers can be enabled/disabled via CMake with -DENABLE_VALIDATION_LAYERS=ON/OFF
#ifdef ENABLE_VALIDATION_LAYERS
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

// defs
bool init_vulkan(int windowWidth, int windowHeight);
void shutdown_vulkan();

// public

bool VKimp_Init(glimpParms_t parms) {
   	common->Printf( "----- Initializing Vulkan -----\n" );

    SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if(parms.fullScreen && parms.borderless) {
		flags |= SDL_WINDOW_FULLSCREEN;
	}
    if ( r_windowResizable.GetBool() ) {
		flags |= SDL_WINDOW_RESIZABLE;
	}

	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, WINDOW_TITLE);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, parms.width);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, parms.height);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, flags);
    window = SDL_CreateWindowWithProperties(props);

	if (!init_vulkan(parms.width, parms.height)) {
	    common->Warning("Failed to create vulkan instance\n");
		shutdown_vulkan();
	    return false;
	}

	// TODO: Render loop

    return true;
};

// private
//
//

static inline bool _chk(VkResult result, int line_no) {
	if (result != VK_SUCCESS) {
		common->Warning("[vk] Vulkan call returned an error (result=%d)\n", result);
		return false;
	}
	return true;
}
#define chk(result) _chk(result, __LINE__)

void shutdown_vulkan() {
    if (depthImage && depthImageAllocation) {
        vmaDestroyImage(allocator, depthImage, depthImageAllocation);
        depthImage = VK_NULL_HANDLE;
        depthImageAllocation = VK_NULL_HANDLE;
    }
    if (swapchain) {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
    }
    if (surface) {
        SDL_Vulkan_DestroySurface(instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }
    if (allocator) {
        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
    }
    if (device) {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }
    if (instance) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
}


bool init_vulkan(int windowWidth, int windowHeight) {
    //if (enableValidationLayers && !check_validation_layer_support()) {
    //	fprintf(stderr, "validation layers requested, but not available!");
    //	return false;
    //}

    // Create our vulkan instance, we're requesting a minimum of vk1.3
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = WINDOW_TITLE,
        .apiVersion = VK_API_VERSION_1_3
    };
    uint32_t instanceExtensionsCount{ 0 };
    char const* const* instanceExtensions{ SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount) };
    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = instanceExtensionsCount,
        .ppEnabledExtensionNames = instanceExtensions,
    };
    if (!chk(vkCreateInstance(&instanceCI, nullptr, &instance))) {
        return false;
    }

    // Look for the actual rendering devices
    uint32_t deviceCount{ 0 };
    if (!chk(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr))) {
        return false;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    if (!chk(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()))) {
        return false;
    }

    // Select our rendering device, in this case we'll just pick the first one and wire up a cvar later.
    // TODO: mel <hdr>: Wire up a cvar for device selection
    uint32_t deviceIndex{ 0 };
    VkPhysicalDeviceProperties2 deviceProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    vkGetPhysicalDeviceProperties2(devices[deviceIndex], &deviceProperties);
    common->Printf("[vk] Selected device: %s\n", deviceProperties.properties.deviceName);

    // Select our queue (we're only interested in gfx not compute or raytracing)
    uint32_t queueFamilyCount{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(devices[deviceIndex], &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(devices[deviceIndex], &queueFamilyCount, queueFamilies.data());
    uint32_t queueFamily{ 0 };
    for (size_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamily = i;
            break;
        }
    }

    // Make sure we can use this queue for flipping an image to the display.
    if(!SDL_Vulkan_GetPresentationSupport(instance, devices[deviceIndex], queueFamily)) {
        common->Warning("[vk] Vulkan device queue does not support presenting.\n");
        return false;
    }

    // Request a single queue
    const float qfpriorities{ 1.0f };
    VkDeviceQueueCreateInfo queueCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &qfpriorities
    };

    // Now we'll be requesting a device handle
    const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    // ...it's a little verbose lol, we need to chain our requests together with pNext
    VkPhysicalDeviceVulkan12Features enabledVk12Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .descriptorBindingVariableDescriptorCount = true,
        .runtimeDescriptorArray = true,
        .bufferDeviceAddress = true
    };
    VkPhysicalDeviceVulkan13Features enabledVk13Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enabledVk12Features,
        .synchronization2 = true,
        .dynamicRendering = true,
    };
    VkPhysicalDeviceFeatures enabledVk10Features{
        .samplerAnisotropy = VK_TRUE
    };

    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledVk13Features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCI,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &enabledVk10Features
    };
    if (!chk(vkCreateDevice(devices[deviceIndex], &deviceCI, nullptr, &device))) {
        return false;
    }

    // Setup VMA
    VmaVulkanFunctions vkFunctions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkCreateImage = vkCreateImage
    };
    VmaAllocatorCreateInfo allocatorCI{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = devices[deviceIndex],
        .device = device,
        .pVulkanFunctions = &vkFunctions,
        .instance = instance
    };
    if(!chk(vmaCreateAllocator(&allocatorCI, &allocator))) {
        return false;
    }

    // We need a surface to present, ask SDL for one!
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
        common->Warning("[vk] Failed to create vulkan surface (err=%s)\n", SDL_GetError());
        return false;
	}
    // (what did we just make?)
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    if (!chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devices[deviceIndex], surface, &surfaceCaps))) {
        return false;
    }

    // Okay create the swapchain
    VkExtent2D swapchainExtent{ surfaceCaps.currentExtent };
    if (surfaceCaps.currentExtent.width == 0xFFFFFFFF) {
        swapchainExtent = { .width = static_cast<uint32_t>(windowWidth), .height = static_cast<uint32_t>(windowHeight) };
    }
    const VkFormat imageFormat{ VK_FORMAT_B8G8R8A8_SRGB };
    VkSwapchainCreateInfoKHR swapchainCI{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = surfaceCaps.minImageCount,
        .imageFormat = imageFormat,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent{.width = swapchainExtent.width, .height = swapchainExtent.height },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR
    };
    if (!chk(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain))) {
        return false;
    }

    // Request the images for the swapchain
    uint32_t imageCount{ 0 };
    if (!chk(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr))) {
        return false;
    }
    swapchainImages.resize(imageCount);
    if (!chk(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data()))) {
        return false;
    }
    swapchainImageViews.resize(imageCount);

    // This is for depth buffer but we don't need it. It's here for the future!
    // We use D24S8 in opengl so try and match that in case we need it.
    std::vector<VkFormat> depthFormatList{ /*VK_FORMAT_D32_SFLOAT_S8_UINT,*/ VK_FORMAT_D24_UNORM_S8_UINT };
    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };
    for (VkFormat& format : depthFormatList) {
        VkFormatProperties2 formatProperties{ .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
        vkGetPhysicalDeviceFormatProperties2(devices[deviceIndex], format, &formatProperties);
        if (formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            depthFormat = format;
            break;
        }
    }
    VkImageCreateInfo depthImageCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depthFormat,
        .extent{.width = static_cast<uint32_t>(windowWidth), .height = static_cast<uint32_t>(windowHeight), .depth = 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo allocCI{
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    if (!chk(vmaCreateImage(allocator, &depthImageCI, &allocCI, &depthImage, &depthImageAllocation, nullptr))) {
        return false;
    }

    // Okay we want an accessor (view) for this depth buffer
    VkImageViewCreateInfo depthViewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = depthFormat,
        .subresourceRange{ .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1 }
    };
    if (!chk(vkCreateImageView(device, &depthViewCI, nullptr, &depthImageView))) {
        return false;
    }

    // Okay handle our quad mesh - we stack the vertices and indices into the same buffer because it's faster
    VkDeviceSize vBufSize{ sizeof(Vertex) * vertices.size() };
    VkDeviceSize iBufSize{ sizeof(uint16_t) * vertex_indices.size() };
    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = vBufSize + iBufSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    };

    // Make sure the buffer is stored in VRAM, but accessible via the CPU if needed
    VmaAllocationCreateInfo vBufferAllocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    VmaAllocationInfo vBufferAllocInfo{};
    if (!chk(vmaCreateBuffer(allocator, &bufferCI, &vBufferAllocCI, &vBuffer, &vBufferAllocation, &vBufferAllocInfo))) {
        return false;
    }

    // We get a persistent mapped buffer so we can copy data directly into VRAM (Thanks VMA_ALLOCATION_CREATE_MAPPED_BIT)
    memcpy(vBufferAllocInfo.pMappedData, vertices.data(), vBufSize);
    memcpy(((char*)vBufferAllocInfo.pMappedData) + vBufSize, vertex_indices.data(), iBufSize);

    // Request buffers for each frame we want to prepare ahead of time/present
    for (auto i = 0; i < maxFramesInFlight; i++) {
        VkBufferCreateInfo uBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(ShaderData),
            .usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        };
        VmaAllocationCreateInfo uBufferAllocCI{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };
        if (!chk(vmaCreateBuffer(allocator, &uBufferCI, &uBufferAllocCI, &shaderDataBuffers[i].buffer, &shaderDataBuffers[i].allocation, &shaderDataBuffers[i].allocationInfo))) {
            return false;
        }
        VkBufferDeviceAddressInfo uBufferBdaInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = shaderDataBuffers[i].buffer
        };
        shaderDataBuffers[i].deviceAddress = vkGetBufferDeviceAddress(device, &uBufferBdaInfo);
    }

    // Create our sync objects
    //
    // Classic hold until ready
    VkSemaphoreCreateInfo semaphoreCI{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    // Signal when ready
    VkFenceCreateInfo fenceCI{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    for (auto i = 0; i < maxFramesInFlight; i++) {
        if (!chk(vkCreateFence(device, &fenceCI, nullptr, &fences[i]))) {
            return false;
        }
        if (!chk(vkCreateSemaphore(device, &semaphoreCI, nullptr, &imageAcquiredSemaphores[i]))) {
            return false;
        }
    }
    renderCompleteSemaphores.resize(swapchainImages.size());
    for (auto& semaphore : renderCompleteSemaphores) {
        if (!chk(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore))) {
            return false;
        }
    }

    // Create the command buffer
    VkCommandPoolCreateInfo commandPoolCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamily
    };
    if (!chk(vkCreateCommandPool(device, &commandPoolCI, nullptr, &commandPool))) {
        return false;
    }

    // Alloc one per frame in flight
    VkCommandBufferAllocateInfo cbAllocCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .commandBufferCount = maxFramesInFlight
    };
    if (!chk(vkAllocateCommandBuffers(device, &cbAllocCI, commandBuffers.data()))) {
        return false;
    }

    // Chapter: Loading textures goes here ...
    //
    //

    // Load shaders!
    //

#if 0
    // Create the connection between slang and our app
    slang::createGlobalSession(slangGlobalSession.writeRef());

    // Create the compile scope
    // ...I had issues with struct assignments...


    slang::TargetDesc targetDesc;
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = slangGlobalSession->findProfile("spirv_1_4");


    slang::CompilerOptionEntry compilerOptionEntry;
    compilerOptionEntry.name = slang::CompilerOptionName::EmitSpirvDirectly;
    compilerOptionEntry.value.kind = slang::CompilerOptionValueKind::Int;
    compilerOptionEntry.value.intValue0 = 1;

    std::array<slang::TargetDesc, 1> slangTargets = { targetDesc };
    std::array<slang::CompilerOptionEntry, 1> slangOptions = { compilerOptionEntry };
    slang::SessionDesc slangSessionDesc;

    slangSessionDesc.targets = slangTargets.data();
    slangSessionDesc.targetCount = SlangInt(slangTargets.size());
    slangSessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
    slangSessionDesc.compilerOptionEntries = slangOptions.data();
    slangSessionDesc.compilerOptionEntryCount = uint32_t(slangOptions.size());

    Slang::ComPtr<slang::ISession> slangSession;
    slangGlobalSession->createSession(slangSessionDesc, slangSession.writeRef());

    // Load the shader from a file path
    const char* shaderTxt =
        R"SHADER(
        /* Copyright (c) 2025-2026, Sascha Willems
         *
         * SPDX-License-Identifier: MIT
         *
         */

        struct VSInput {
            float2 Pos;
            float3 Color;
	//float2 UV;
        };

        Sampler2D textures[];

        struct ShaderData {
            float4x4 model;
            float4x4 view;
            float4x4 proj;
        };

        struct VSOutput {
            float4 Pos : SV_POSITION;
            float3 Color;
            //float2 UV;
            uint32_t InstanceIndex;
        };

        [shader("vertex")]
        VSOutput main(VSInput input, uniform ShaderData *shaderData, uint instanceIndex : SV_VulkanInstanceID) {
            VSOutput output;
            output.Color = input.Color;
            output.Pos = float4(input.Pos.x, input.Pos.y, 0, 1);
            output.InstanceIndex = instanceIndex;
            return output;
        }

        [shader("fragment")]
        float4 main(VSOutput input) {
            return input.Color;
        })SHADER";
    //
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    Slang::ComPtr<slang::IModule> slangModule{
        slangSession->loadModuleFromSource("triangle", "base/slang/shader.slang", (slang::IBlob*)shaderTxt, diagnosticsBlob.writeRef())
    };
    diagnoseIfNeeded(diagnosticsBlob);
    if (!slangModule)
    {
        return false;
    }
    Slang::ComPtr<ISlangBlob> spirv;
    slangModule->getTargetCode(0, spirv.writeRef());
#else


#endif

    void* glslBuffer = NULL;
	int glslBufferLength = fileSystem->ReadFile("spirv/shader.spv", &glslBuffer);
	if (glslBufferLength == -1) {
	    common->Warning("[vk] Shader file not found!\n");
		return false;
	}

    // Create a shader module from that shader
    VkShaderModuleCreateInfo shaderModuleCI{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = (size_t)glslBufferLength,
        .pCode = (uint32_t*)glslBuffer
    };
    VkShaderModule shaderModule{};
    if (!chk(vkCreateShaderModule(device, &shaderModuleCI, nullptr, &shaderModule))) {
        return false;
    }

    fileSystem->FreeFile(glslBuffer);

    // Gfx pipeline!
    // How we move stuff to the shader I believe
    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .size = sizeof(VkDeviceAddress)
    };
    VkPipelineLayoutCreateInfo pipelineLayoutCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayoutTex,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };
    if (!chk(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout))) {
        return false;
    }

    // We're creating vertex bindings and defining our vertex attribute layout
    VkVertexInputBindingDescription vertexBinding{
         .binding = 0,
         .stride = sizeof(Vertex),
         .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
    std::vector<VkVertexInputAttributeDescription> vertexAttributes{
        { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT },
        { .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, color) },
        //{ .location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, uv) },
    };

    VkPipelineVertexInputStateCreateInfo vertexInputState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertexBinding,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size()),
        .pVertexAttributeDescriptions = vertexAttributes.data(),
    };
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
        { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage = VK_SHADER_STAGE_VERTEX_BIT,
          .module = shaderModule, .pName = "main"},
        { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
          .module = shaderModule, .pName = "main" }
    };

    // Viewport state!
    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };
    std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates.data()
    };
    VkPipelineDepthStencilStateCreateInfo depthStencilState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL
    };
    VkPipelineRenderingCreateInfo renderingCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &imageFormat,
        .depthAttachmentFormat = depthFormat
    };

    // Set some sane defaults
    VkPipelineColorBlendAttachmentState blendAttachment{
        .colorWriteMask = 0xF
    };
    VkPipelineColorBlendStateCreateInfo colorBlendState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &blendAttachment
    };
    VkPipelineRasterizationStateCreateInfo rasterizationState{
         .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
         .lineWidth = 1.0f
    };
    VkPipelineMultisampleStateCreateInfo multisampleState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    // and make!
    VkGraphicsPipelineCreateInfo pipelineCI{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingCI,
        .stageCount = 2,
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = &depthStencilState,
        .pColorBlendState = &colorBlendState,
        .pDynamicState = &dynamicState,
        .layout = pipelineLayout
    };
    if (!chk(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline))) {
        return false;
    }

	return true;
}
