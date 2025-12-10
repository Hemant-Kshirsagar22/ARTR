# depth recommonded changes
1. In createRenderPass() : vkSubpassDescription.colorAttachmentCount = 1;
2. In createFrameBuffers() : move declaration and memset of vkImageView_attachments_array and declaration, memset and initialization of VkFramebufferCreateInfo into below loop.

# actual changes for depth
1. In createImagesAndImageView() : create and allocate memory for depth image