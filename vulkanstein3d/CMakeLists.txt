cmake_minimum_required (VERSION 3.8)

add_subdirectory ("Shaders")

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_executable (vulkanstein3d
    "Main.cpp"
    "App/Input.cpp"
    "App/Window.cpp"
    "Game/Assets.cpp"
    "Game/MeshGenerator.cpp"
    "Game/PlayerController.cpp"
    "Rendering/Buffer.cpp"
    "Rendering/Device.cpp"
    "Rendering/Instance.cpp" 
    "Rendering/MaterialBuilder.cpp"
    "Rendering/PipelineBuilder.cpp"
    "Rendering/Swapchain.cpp"
    "Rendering/Texture.cpp"
    "Wolf3dLoaders/Loaders.cpp"
)

target_include_directories(vulkanstein3d PRIVATE
    . ..
    ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(vulkanstein3d PRIVATE    
    glfw
    glm::glm    	
    spdlog::spdlog 
    spdlog::spdlog_header_only
    unofficial::vulkan-memory-allocator::vulkan-memory-allocator
    ${Vulkan_LIBRARY}
    xbrz
)

target_compile_definitions(vulkanstein3d PUBLIC VULKAN_DEBUG)

if (WIN32)
    target_compile_definitions(vulkanstein3d PUBLIC VK_USE_PLATFORM_WIN32_KHR NOMINMAX)
endif (WIN32)

set_target_properties(vulkanstein3d PROPERTIES CXX_STANDARD 20)

# https://docs.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-170#enable-addresssanitizer-for-windows-and-linux
option(ASAN_ENABLED "Build this target with AddressSanitizer" ON)

#if(ASAN_ENABLED)
  #if(MSVC)
    #target_compile_options(vulkanstein3d PUBLIC /fsanitize=address)
  #else()
    #target_compile_options(vulkanstein3d PUBLIC -fsanitize=address <additional-options>)
    #target_link_options(vulkanstein3d PUBLIC -fsanitize=address)
  #endif()
#endif()
