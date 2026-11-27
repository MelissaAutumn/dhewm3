# Vulkan support

Vulkan 1.3 support has been added to enable the use of HDR displays. OpenGL does not have native support for additional colorspaces, and while EGL contains some extensions they are not supported on my platform (Linux via Mesa/AMD.)

Most of the rendering is still done via OpenGL 4 in compatibility mode, but by leveraging the Vulkan extensions `VK_KHR_external_memory` and `VK_KHR_external_semaphore` and the OpenGL extension `GL_EXT_semaphore` we can allocate memory with Vulkan and use it with OpenGL which allows us to share the image.

Applying HDR rendering tweaks from [learnopengl's HDR](https://learnopengl.com/Advanced-Lighting/HDR) article in OpenGL we can render into a HDR framebuffer, and pass it to Vulkan which will render the image out to an HDR display.

The Vulkan code is implemented in `sys/vkimp.cpp` and mostly comes from [howtovulkan](https://www.howtovulkan.com) and [stevelittlefish's c_vulkan_sdl3 sample](https://github.com/stevelittlefish/c_vulkan_sdl3).

## Quirks

`sys/glimp.cpp` now contains two SDL_Window pointers, `glWindow` which is where the hidden OpenGL window is created, and the regular ol' `window` which is where the Vulkan window is created if `VULKAN_DISPLAY` cmake build option is enabled. If that build option is disabled then after `glWindow` is created it will be passed to `window`.

I had a lot of issues getting slang to work correctly so I ended up ditching runtime compilation. You'll need to compile `shader.slang` to spirv manually with: `slang base/slang/shader.slang`. 

## Requirements

* vulkan sdk
* shader-slang (arch package)
* vma (included into source tree here)
