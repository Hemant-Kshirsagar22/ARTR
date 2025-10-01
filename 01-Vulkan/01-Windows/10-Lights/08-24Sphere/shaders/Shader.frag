#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 out_transformedNormal;
layout(location = 1) in vec3 out_lightDirection;
layout(location = 2) in vec3 out_viewerVector;

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

	// key press related uniform
	uint lKeyIsPressed;
}uMyUniformData;

layout(push_constant) uniform PushConstantData_Material {
    // material related uniforms
    vec4 materialAmbient;
    vec4 materialDiffuse;
    vec4 materialSpecular;
    
	float materialShininess;
} pushConstantData_Material;

void main(void)
{
	// code
	vec3 phong_ads_light;

	if(uMyUniformData.lKeyIsPressed == 1)
	{
		vec3 normalized_tranformed_normals = normalize(out_transformedNormal);
		vec3 normalized_light_direction = normalize(out_lightDirection);
		vec3 normalized_viewer_vector = normalize(out_viewerVector);

		vec3 ambientLight = vec3(uMyUniformData.lightAmbent) * vec3(pushConstantData_Material.materialAmbient);

		vec3 diffuse_light = vec3(uMyUniformData.lightDiffuse) * vec3(pushConstantData_Material.materialDiffuse) * max(dot(normalized_light_direction, normalized_tranformed_normals), 0.0);

		vec3 reflectionVector = reflect(-normalized_light_direction, normalized_tranformed_normals);

		vec3 specularLight = vec3(uMyUniformData.lightSpecular) * vec3(pushConstantData_Material.materialSpecular) * pow(max(dot(reflectionVector, normalized_viewer_vector), 0.0), pushConstantData_Material.materialShininess);

		phong_ads_light = ambientLight + diffuse_light + specularLight;
	}
	else
	{
		phong_ads_light = vec3(1.0f, 1.0f, 1.0f);
	}

	FragColor = vec4(phong_ads_light, 1.0f);
}
