#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
	mat4 viewProjection;
} cameraData;

layout(std140, set = 0, binding = 1) readonly buffer storageBuffer {
	mat4 model[];
} ObjectData;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexKa;
layout(location = 2) in vec3 vertexKd;
layout(location = 3) in vec3 vertexKs;
layout(location = 4) in float e;
layout(location = 5) in vec2 vertexTexCoord;
layout(location = 6) in vec3 vertexNormal;

layout(location = 0) out vec3 fragKa;
layout(location = 1) out vec3 fragKd;
layout(location = 2) out vec3 fragKs;
layout(location = 3) out float fragE;
layout(location = 4) out vec2 fragTexCoord;
layout(location = 5) out vec3 fragPosCameraSpace;
layout(location = 6) out vec3 fragNormalCameraSpace;

void main()
{
	gl_Position = cameraData.viewProjection * ObjectData.model[gl_InstanceIndex] * vec4(vertexPosition, 1.0);
	fragPosCameraSpace = vec3(cameraData.view * (ObjectData.model[gl_InstanceIndex] * vec4(vertexPosition, 1.0)));
	fragNormalCameraSpace = vec3(transpose(inverse(cameraData.view * ObjectData.model[gl_InstanceIndex])) * vec4(vertexNormal, 0.0));
	fragKa = vertexKa;
	fragKd = vertexKd;
	fragKs = vertexKs;
	fragE = e;
	fragTexCoord = vertexTexCoord;
}