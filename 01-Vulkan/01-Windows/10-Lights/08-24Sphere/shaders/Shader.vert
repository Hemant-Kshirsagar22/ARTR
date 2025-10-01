#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 0) out vec3 out_transformedNormal;
layout(location = 1) out vec3 out_lightDirection;
layout(location = 2) out vec3 out_viewerVector;

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

	// key press related uniform
	uint lKeyIsPressed;
}uMyUniformData;

void main(void)
{
	// code
	if(uMyUniformData.lKeyIsPressed == 1)
	{
		vec4 eyeCoordinate = uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vPosition;

		out_transformedNormal = mat3(uMyUniformData.viewMatrix * uMyUniformData.modelMatrix) * vNormal;
		out_lightDirection = vec3(uMyUniformData.lightPosition - eyeCoordinate);
		out_viewerVector = -eyeCoordinate.xyz;
	}

	gl_Position = uMyUniformData.projectionMatrix * uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vPosition;
}
