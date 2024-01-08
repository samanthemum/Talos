#version 450

layout(location = 0) in vec3 forward;
layout(set = 1, binding = 0) uniform samplerCube material;

layout(location = 0) out vec4 outColor;

void main() {

	outColor = texture(material, forward);
}

