#version 450

layout(location = 0) in vec3 fragKa;
layout(location = 1) in vec3 fragKd;
layout(location = 2) in vec3 fragKs;
layout(location = 3) in float fragE;
layout(location = 4) in vec2 fragTexCoord;
layout(location = 5) in vec3 fragPosCameraSpace;
layout(location = 6) in vec3 fragNormalCameraSpace;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D tex;
layout(std140, set = 1, binding = 0) uniform LightData {
	vec4 numLights;
	vec4 lightPositions[16];
	vec4 lightColors[16];
} lighting;

void main() {

	// map kd
	vec3 kd = fragKd;
	if(fragTexCoord.x >= 0) {
		kd = vec3(texture(tex, fragTexCoord));
	}

	vec3 finalColor = vec3(0.0f, 0.0f, 0.0f);
	for(int i = 0; i < lighting.numLights.x; i++) {
		vec3 n = normalize(fragNormalCameraSpace);
		vec3 lightDir = normalize(vec3(lighting.lightPositions[i]) - fragPosCameraSpace);
		vec3 eye = normalize(vec3(0.0f, 0.0f, 0.0f) - fragPosCameraSpace);
		vec3 h = normalize(lightDir + eye);

		vec3 diffuse = kd * max(0, dot(lightDir, n));
		vec3 shiny = fragKs * pow(max(0, dot(h, n)), fragE);
		finalColor = finalColor + (vec3(lighting.lightColors[i]) * (kd + diffuse + shiny));
	}

	outColor = vec4(finalColor, 1.0f);
}

