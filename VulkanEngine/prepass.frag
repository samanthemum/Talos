#version 450

layout(location = 0) in vec3 fragKa;
layout(location = 1) in vec3 fragKd;
layout(location = 2) in vec3 fragKs;
layout(location = 3) in float fragE;
layout(location = 4) in vec2 fragTexCoord;
layout(location = 5) in vec3 fragPosCameraSpace;
layout(location = 6) in vec3 fragNormalCameraSpace;

layout(location = 0) out vec4 albedo;
layout(location = 1) out vec4 normal;

layout(set = 1, binding = 0) uniform sampler2D tex;

void main() {

	// map kd
	vec3 kd = fragKd;
	if(fragTexCoord.x >= 0) {
		kd = vec3(texture(tex, fragTexCoord));
	}

	albedo = vec4(kd, 1.0f);
	normal = vec4(fragNormalCameraSpace, 0.0);
}

