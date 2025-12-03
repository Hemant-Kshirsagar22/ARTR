#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

layout(location = 0) out vec2 out_TexCoord;

layout(location = 1) out vec3 out_transformedNormal;
layout(location = 2) out vec3 out_lightDirection;
layout(location = 3) out vec3 out_viewerVector;

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

    int iKeyIsPressed;
}uMyUniformData;

void main(void)
{
	// code
	if(uMyUniformData.iKeyIsPressed == 1)
	{
		vec4 eyeCoordinate = uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vPosition;

		out_transformedNormal = mat3(uMyUniformData.viewMatrix * uMyUniformData.modelMatrix) * vNormal;

		out_lightDirection = vec3(uMyUniformData.lightPosition - eyeCoordinate);
		out_viewerVector = -eyeCoordinate.xyz;
	}
	else 
	{
		vec4 eyeCoordinate = vec4(0.0f);
		out_transformedNormal = vec3(0.0f);
		out_lightDirection = vec3(0.0f);
		out_viewerVector = vec3(0.0f);
	}

	gl_Position = uMyUniformData.projectionMatrix * uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vPosition;
	out_TexCoord = vTexCoord;
}
