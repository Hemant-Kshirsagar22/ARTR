#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 0) out vec3 out_transformedNormal;
layout(location = 1) out vec3 out_lightDirection[3];
layout(location = 4) out vec3 out_viewerVector;

layout(binding = 0) uniform MyUniformData
{
	// matrices related uniform
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	// lighting related uniform
	vec4 lightAmbient[3];
    vec4 lightDiffuse[3];
    vec4 lightSpecular[3];
    vec4 lightPosition[3];

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
		vec4 eyeCoordinate = uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vPosition;

		out_transformedNormal = mat3(uMyUniformData.viewMatrix * uMyUniformData.modelMatrix) * vNormal;
		
		out_lightDirection[0] = vec3(uMyUniformData.lightPosition[0] - eyeCoordinate);
		out_lightDirection[1] = vec3(uMyUniformData.lightPosition[1] - eyeCoordinate);
		out_lightDirection[2] = vec3(uMyUniformData.lightPosition[2] - eyeCoordinate);
		out_viewerVector = -eyeCoordinate.xyz;
	}
	else
	{
		out_transformedNormal = vec3(0.0f);
		out_viewerVector = vec3(0.0f);
		out_lightDirection[0] = vec3(0.0f);
		out_lightDirection[1] = vec3(0.0f);
		out_lightDirection[2] = vec3(0.0f);
	}

	gl_Position = uMyUniformData.projectionMatrix * uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vPosition;
}
