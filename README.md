# README.md

## Build

### Version
- CMake 3.28.3
- C++ 23
- SPIR-V 1.6
- Vulkan 1.3.250

## Dependencies

### Non Third party

#### packager
- **Location**: [./package](./package)
- **Information**:   [README.md](./package/README.md)

### Third party

#### glfw
- **Location**: [./Vendor/glfw](./Vendor/glfw)
- **Repository**: [https://github.com/glfw/glfw](https://github.com/glfw/glfw)
- **Submodule**: Yes

#### glm
- **Location**: [./Vendor/glm](./Vendor/glm)
- **Repository**: [https://github.com/g-truc/glm](https://github.com/g-truc/glm)
- **Submodule**: Yes

### stb
- **Note**: stb is not a submodule because it is only 3 files (4 including the implementation file) and therefore is included directly
in the repository.
- **Location**: [./Vendor/stb](./Vendor/stb)
    - stb_image.h
    - stb_image_write.h
    - stb_image_resize2.h
- **Repository**: [https://github.com/nothings/stb](https://github.com/nothings/stb)
- **Submodule**: No 


#### VulkanSDK
- **Note**: You will have to install the VulkanSDK yourself from their [website](https://vulkan.lunarg.com/), then copy it to the VulkanSDK location.
- **Location**: [./Vendor/VulkanSDK](./Vendor/VulkanSDK)
- **Repository**: [https://github.com/KhronosGroup/VulkanSDK](https://github.com/KhronosGroup/VulkanSDK)
- **Submodule**: No 
