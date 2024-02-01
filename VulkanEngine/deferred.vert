#version 450

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragPosCameraSpace;
layout(location = 2) out vec3 fragPosWorldSpace;

const vec2 screen_corners[6] = vec2[] (
	vec2(-1.0, -1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, 1.0),
	vec2(1.0, 1.0),
	vec2(1.0, -1.0),
	vec2(-1.0, -1.0)
);

void main()
{
	vec2 pos = screen_corners[gl_VertexIndex];
	gl_Position = vec4(pos, 0.0, 1.0);
	fragTexCoord = (pos + 1.0) / 2.0;
}