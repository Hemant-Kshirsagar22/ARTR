#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec4 vPosition;

layout(binding = 0) uniform MyUniformData
{
	mat4 mvpMatrix;
}uMyUniformData;

void main(void)
{
	// code
	gl_Position = uMyUniformData.mvpMatrix * vPosition;
	
    gl_Position.y = -gl_Position.y;
}
