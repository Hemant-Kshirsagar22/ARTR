#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec3 out_diffuse_light;
layout(location = 0) out vec4 FragColor;
void main(void)
{
	// code
	FragColor = vec4(out_diffuse_light, 1.0f);
}
