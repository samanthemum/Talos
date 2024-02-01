#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
	mat4 viewProjection;
} cameraData;

layout(std140, set = 0, binding = 1) uniform LightData {
	vec4 numLights;
	vec4 lightPositions[16];
	vec4 lightColors[16];
} lighting;

layout(set = 1, binding = 0) uniform sampler2D albedoBuffer;
layout(set = 1, binding = 1) uniform sampler2D normalBuffer;
layout(set = 1, binding = 2) uniform sampler2D positionBuffer;

void main() {

	// map kd
	vec3 kd = vec3(0.0f, 0.0f, 0.0f);
	if(fragTexCoord.x >= 0) {
		kd = vec3(texture(albedoBuffer, fragTexCoord));
	}

	// Convert to camera space because that's what the lights are in?
	vec3 n = normalize(vec3(cameraData.view * texture(normalBuffer, fragTexCoord)));
	vec3 finalColor = vec3(0.0f, 0.0f, 0.0f);
	vec3 fragPosWorldSpace = vec3(texture(positionBuffer, fragTexCoord));
	vec3 fragPosCameraSpace = vec3(cameraData.view * vec4(fragPosWorldSpace, 1.0f));
	for(int i = 0; i < lighting.numLights.x; i++) {
		vec3 lightDir = normalize(vec3(lighting.lightPositions[i]) - fragPosCameraSpace);
		vec3 eye = normalize(vec3(0.0f, 0.0f, 0.0f) - fragPosCameraSpace);
		vec3 h = normalize(lightDir + eye);

		vec3 diffuse = kd * max(0, dot(lightDir, n));
		finalColor = finalColor + (vec3(lighting.lightColors[i]) * (kd + diffuse));
	}

	outColor = vec4(finalColor, 1.0);
}

