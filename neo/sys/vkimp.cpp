#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "renderer/tr_local.h"
#include "idlib/math/Vector.h"
#include "idlib/math/Matrix.h"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct {
    uint32_t graphics_family;
    uint32_t present_family;
	bool has_graphics_family;
	bool has_present_family;
} QueueFamilyIndices;

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
	uint32_t formats_count;
	uint32_t present_modes_count;
	VkSurfaceFormatKHR* formats;
	VkPresentModeKHR* present_modes;
} SwapChainSupportDetails;

typedef struct {
	idVec2 pos;
	idVec3 color;
} Vertex;

typedef struct {
	idMat4 model;
	idMat4 view;
	idMat4 proj;
} UniformBufferObject;

// Validation layers can be enabled/disabled via CMake with -DENABLE_VALIDATION_LAYERS=ON/OFF
#ifdef ENABLE_VALIDATION_LAYERS
const bool enable_validation_layers = true;
#else
const bool enable_validation_layers = false;
#endif

#define NUM_VALIDATION_LAYERS 1
const char* validation_layers[NUM_VALIDATION_LAYERS] = {
	"VK_LAYER_KHRONOS_validation"
};

#define NUM_DEVICE_EXTENSIONS 1
const char* device_extensions[NUM_DEVICE_EXTENSIONS] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkInstance instance = {0};
VkDebugUtilsMessengerEXT debug_messenger = {0};
VkSurfaceKHR surface = {0};
// Phyiical device is the GPU
VkPhysicalDevice physical_device = VK_NULL_HANDLE;
// Logical device is the interface to the physical device
VkDevice device = {0};

VkQueue graphics_queue = {0};
VkQueue present_queue = {0};

VkSwapchainKHR swap_chain = {0};
uint32_t swap_chain_images_count = 0;
VkImage *swap_chain_images = NULL;
VkFormat swap_chain_image_format = VK_FORMAT_UNDEFINED;
VkExtent2D swap_chain_extent = {0};
uint32_t swap_chain_image_views_count = 0;
VkImageView *swap_chain_image_views = NULL;
uint32_t swap_chain_framebuffers_count = 0;
VkFramebuffer *swap_chain_framebuffers = NULL;

VkRenderPass render_pass = {0};
VkDescriptorSetLayout descriptor_set_layout = {0};
VkPipelineLayout pipeline_layout = {0};
VkPipeline graphics_pipeline = {0};

VkDescriptorPool descriptor_pool = {0};
VkDescriptorSet descriptor_sets[MAX_FRAMES_IN_FLIGHT] = {0};

VkCommandPool command_pool = {0};
const uint32_t command_buffers_count = MAX_FRAMES_IN_FLIGHT;
VkCommandBuffer command_buffers[MAX_FRAMES_IN_FLIGHT] = {0};

// Semaphores: need enough to avoid reuse before presentation completes
// Use max of MAX_FRAMES_IN_FLIGHT and swapchain image count
uint32_t semaphores_count = 0;
VkSemaphore* image_available_semaphores = NULL;
VkSemaphore* render_finished_semaphores = NULL;

// Per-frame fences
const uint32_t in_flight_fences_count = MAX_FRAMES_IN_FLIGHT;
VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT] = {0};

uint32_t current_frame = 0;
bool framebuffer_resized = false;

#define NUM_VERTICES 4
Vertex vertices[NUM_VERTICES] = {
    {.pos=idVec2(-0.5f, -0.5f), .color=idVec3(1.0f, 0.0f, 0.0f)},
    {.pos=idVec2(0.5f, -0.5f), .color=idVec3(0.0f, 1.0f, 0.0f)},
    {.pos=idVec2(0.5f, 0.5f), .color=idVec3(0.0f, 0.0f, 1.0f)},
    {.pos=idVec2(-0.5f, 0.5f), .color=idVec3(1.0f, 1.0f, 1.0f)},
};

#define NUM_VERTEX_INDICES 6
uint16_t vertex_indices[NUM_VERTEX_INDICES] = {
	0, 1, 2, 2, 3, 0
};

VkBuffer vertex_buffer = {0};
VkDeviceMemory vertex_buffer_memory = {0};

VkBuffer index_buffer = {0};
VkDeviceMemory index_buffer_memory = {0};

VkBuffer uniform_buffers[MAX_FRAMES_IN_FLIGHT] = {0};
VkDeviceMemory uniform_buffers_memory[MAX_FRAMES_IN_FLIGHT] = {0};
void* uniform_buffers_mapped[MAX_FRAMES_IN_FLIGHT] = {0};


// defs
bool check_validation_layer_support();
char const * const * get_required_extensions(uint32_t* count);

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
	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "vkWindow");
	//SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_UNDEFINED_DISPLAY(selectedDisplay));
	//SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_UNDEFINED_DISPLAY(selectedDisplay));
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, parms.width);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, parms.height);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, flags);

    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, flags);
    window = SDL_CreateWindowWithProperties(props);

	common->Printf("Created vulkan window: %p\n", window);

    return true;
};

// private

bool check_validation_layer_support() {
	uint32_t layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, NULL);

	VkLayerProperties* available_layers = static_cast<VkLayerProperties*>(malloc(sizeof(VkLayerProperties) * layer_count));
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

	for (int i = 0; i < NUM_VALIDATION_LAYERS; i++) {
		bool layer_found = false;

		for (int j = 0; j < layer_count; j++) {
			if (strcmp(validation_layers[i], available_layers[j].layerName) == 0) {
				layer_found = true;
				break;
			}
		}

		if (!layer_found) {
			return false;
		}
	}

	return true;
}

char const * const * get_required_extensions(uint32_t* count) {
	/*
	 * If validation layers are enabled, we need to request the VK_EXT_DEBUG_UTILS_EXTENSION_NAME extension
	 * as well as the extensions required by GLFW.
	 *
	 * Note: calling this repeatedly could cause a memory leak as we create a new array each time
	 * if validation layers are enabled.
	 */
	assert(count != NULL);

	// Get the required extensions from GLFW and set count to the number of extensions
	char const * const * sdl_extensions = SDL_Vulkan_GetInstanceExtensions(count);

	if (sdl_extensions == NULL) {
		fprintf(stderr, "Failed to get required extensions from GLFW\n");
		exit(1);
	}

	if (!enable_validation_layers) {
		return sdl_extensions;
	}

	// If validation layers are enabled, add the debug utils extension
	const char** extensions = static_cast<const char**>(malloc(sizeof(const char*) * (*count + 1)));
	memcpy(extensions, sdl_extensions, sizeof(const char*) * *count);
	extensions[*count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	*count += 1;

	return (char const * const *) extensions;
}
