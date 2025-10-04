#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 out_transformedNormal;
layout(location = 1) in vec3 out_lightDirection[3];
layout(location = 4) in vec3 out_viewerVector;

layout(location = 0) out vec4 FragColor;

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
	vec3 phong_ads_light = vec3(0.0f);

	if(uMyUniformData.lKeyIsPressed == 1)
	{
		vec3 normalized_tranformed_normals = normalize(out_transformedNormal);
		
		vec3 normalized_light_direction[3];
		normalized_light_direction[0] = normalize(out_lightDirection[0]);
		normalized_light_direction[1] = normalize(out_lightDirection[1]);
		normalized_light_direction[2] = normalize(out_lightDirection[2]);

		vec3 normalized_viewer_vector = normalize(out_viewerVector);

		vec3 ambientLight[3];
		vec3 diffuse_light[3];
		vec3 reflectionVector[3];
		vec3 specularLight[3];

		for(int i = 0; i < 3; i++)
		{
			ambientLight[i] = vec3(uMyUniformData.lightAmbient[i]) * vec3(uMyUniformData.materialAmbient);

			diffuse_light[i] = vec3(uMyUniformData.lightDiffuse[i]) * vec3(uMyUniformData.materialDiffuse) * max(dot(normalized_light_direction[i], normalized_tranformed_normals), 0.0);

			reflectionVector[i] = reflect(-normalized_light_direction[i], normalized_tranformed_normals);

			specularLight[i] = vec3(uMyUniformData.lightSpecular[i]) * vec3(uMyUniformData.materialSpecular) * pow(max(dot(reflectionVector[i], normalized_viewer_vector), 0.0), uMyUniformData.materialShininess);

			phong_ads_light = phong_ads_light + ambientLight[i] + diffuse_light[i] + specularLight[i];
		}
	}
	else
	{
		phong_ads_light = vec3(1.0f, 1.0f, 1.0f);
	}

	FragColor = vec4(phong_ads_light, 1.0f);
}
