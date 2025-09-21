#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 out_phongADSLight;
layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform MyUniformData
{
	// matrices related uniform
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	// lighting related uniform
	vec4 lightAmbent;
	vec4 lightDiffuse;
	vec4 lightSpecular;
	vec4 lightPosition;

	vec4 materialAmbient;
	vec4 materialDiffuse;
	vec4 materialSpecular;
	
	float materialShininess;

	// key press related uniform
	uint lKeyIsPressed;
}uMyUniformData;


void main(void)
{
	// code
	if(uMyUniformData.lKeyIsPressed == 1)
	{
		FragColor = vec4(out_phongADSLight, 1.0f);
	}
	else
	{
		FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
}
