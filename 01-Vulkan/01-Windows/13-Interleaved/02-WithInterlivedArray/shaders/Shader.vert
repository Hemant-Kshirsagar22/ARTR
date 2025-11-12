#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexcoord;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_texcoord;

layout(location = 2) out vec3 out_transformedNormal;
layout(location = 3) out vec3 out_lightDirection;
layout(location = 4) out vec3 out_viewerVector;

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
}uMyUniformData;

void main(void)
{
	// code
	vec4 eyeCoordinate = uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vPosition;

	out_transformedNormal = mat3(uMyUniformData.viewMatrix * uMyUniformData.modelMatrix) * vNormal;
	out_lightDirection = vec3(uMyUniformData.lightPosition - eyeCoordinate);
	out_viewerVector = -eyeCoordinate.xyz;

	gl_Position = uMyUniformData.projectionMatrix * uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vPosition;

	out_color = vColor;
	out_texcoord = vTexcoord;
}
