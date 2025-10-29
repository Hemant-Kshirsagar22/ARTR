#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform MyUniformData
{
	mat4 mvpMatrix;
    vec4 numberOfLineSegments;
    vec4 numberOfLineStrips;
    vec4 lineColor;
}myUniformData;

void main(void)
{
	// code
	FragColor = myUniformData.lineColor;
}
