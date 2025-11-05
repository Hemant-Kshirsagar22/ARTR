CLS
C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o ./shaders/Shader.vert.spv ./shaders/Shader.vert
C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o ./shaders/Shader.geom.spv ./shaders/Shader.geom
C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o ./shaders/Shader.frag.spv ./shaders/Shader.frag

cl.exe /c /EHsc /I C:\VulkanSDK\Vulkan\Include VK.cpp
rc.exe VK.rc
link.exe VK.obj VK.res /LIBPATH:C:\VulkanSDK\Vulkan\Lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS
