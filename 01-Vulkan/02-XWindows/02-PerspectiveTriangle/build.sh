clear
g++ -c -I $HOME/VulkanSDK/vulkan/x86_64/include -o VK.o VK.cpp 
g++ -o VK VK.o -lX11 -lm -L $HOME/VulkanSDK/vulkan/x86_64/lib -lvulkan
