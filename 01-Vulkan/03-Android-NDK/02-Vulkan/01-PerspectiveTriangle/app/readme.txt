Steps to create perspective triangle
1. copy 04-Events.
2. Delete window.cpp.
4. Copy windows vulkan perspective triangle vk.cpp there.
5. download binary from https://github.com/khronosGroup/vulkan-validationLayers/releases

shader compiler comamnd
<AndroidSDKPath>/ndk/29.0.14206865/shader-tools/<darwin-x86_64 | os>/glslc -fshader-stage=vertex --target-env=vulkan1.3 -o Shader.vert.spv Shader.vert

/Users/hemantkshirsagar/Hemant/02-college/01-msccs/01-part1/02-sem/AndroidStudio/ndk/29.0.14206865/shader-tools/darwin-x86_64/glslc -fshader-stage=vertex --target-env=vulkan1.3 -o Shader.vert.spv Shader.vert 

/Users/hemantkshirsagar/Hemant/02-college/01-msccs/01-part1/02-sem/AndroidStudio/ndk/29.0.14206865/shader-tools/darwin-x86_64/glslc -fshader-stage=frag --target-env=vulkan1.3 -o Shader.frag.spv Shader.frag
