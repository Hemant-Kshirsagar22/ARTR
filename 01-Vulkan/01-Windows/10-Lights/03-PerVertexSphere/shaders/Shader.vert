#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 0) out vec3 out_phongADSLight;

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
		vec4 eyeCoordinate = uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vPosition;
		vec3 transformedNormal = normalize(mat3(uMyUniformData.viewMatrix * uMyUniformData.modelMatrix) * vNormal);
		vec3 lightDirection = normalize(vec3(uMyUniformData.lightPosition - eyeCoordinate));
		vec3 reflectionVector = reflect(-lightDirection, transformedNormal);
		vec3 viewerVector = normalize(-eyeCoordinate.xyz);

		vec3 ambientLight = vec3(uMyUniformData.lightAmbent) * vec3(uMyUniformData.materialAmbient);
		vec3 diffuseLight = vec3(uMyUniformData.lightDiffuse) * vec3(uMyUniformData.materialDiffuse) * max(dot(lightDirection, transformedNormal), 0.0f);
		vec3 lightSpecular = vec3(uMyUniformData.lightSpecular) * vec3(uMyUniformData.materialSpecular) * pow(max(dot(reflectionVector, viewerVector), 0.0f), uMyUniformData.materialShininess);

		out_phongADSLight = ambientLight + diffuseLight + lightSpecular;
	}
	else
	{
		out_phongADSLight = vec3(1.0f, 1.0f, 1.0f);
	}

	gl_Position = uMyUniformData.projectionMatrix * uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vPosition;
}
