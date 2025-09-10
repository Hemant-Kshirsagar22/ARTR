#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 0) out vec3 out_diffuse_light;

layout(binding = 0) uniform MyUniformData
{
	// matrices related uniform
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	// lighting related uniform
	vec4 lightDiffuse;
	vec4 lightPosition;
	vec4 materialDiffuse;
	
	// key press related uniform
	uint lKeyIsPressed;
}uMyUniformData;

void main(void)
{
	// code
	if(uMyUniformData.lKeyIsPressed == 1)
	{
		vec4 eyeCoordinate = uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vPosition;
		mat3 normalMatrix = mat3(transpose(inverse(uMyUniformData.viewMatrix * uMyUniformData.modelMatrix)));
		vec3 transformedNormal = normalize(normalMatrix * vNormal);
		vec3 lightSource = normalize(vec3(uMyUniformData.lightPosition - eyeCoordinate));

		out_diffuse_light = vec3(uMyUniformData.lightDiffuse) * vec3(uMyUniformData.materialDiffuse) * max(dot(lightSource, transformedNormal), 0.0);
	}
	else
	{
		out_diffuse_light = vec3(1.0, 1.0, 1.0);
	}

	gl_Position = uMyUniformData.projectionMatrix * uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vPosition;
}
