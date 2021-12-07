#version 450

const vec2 uvs[6] = vec2[] 
(
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(0.0, 1.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),
	vec2(0.0, 1.0)
);

struct SpriteData{
	mat4 model;
    vec4 data;
};

layout(set = 0, binding = 1) uniform FrameConstants {
    mat4 view;
    mat4 projection;
    float time;
    float mousexNormalized;
    float mouseyNormalized;
    float padding;
} frame;

layout(set = 0, binding = 2) readonly buffer ObjectBuffer {
	SpriteData sprites[];
} object;

layout(location = 0) out vec3 outUvTile;

void main() {
    vec2 uv = uvs[gl_VertexIndex];

    float sizex = 10.0;
	float sizey = 10.0;
    
    vec3 rightWs = vec3(frame.view[0][0], frame.view[1][0], frame.view[2][0]);
	vec3 upWs = vec3(0.0, 1.0, 0.0);
    
    vec3 pos = vec3(0.0f, 0.0f, 0.0f);
    pos += sizex * (0.5 - uv.x) * rightWs;
    pos += sizey * (0.5 - uv.y) * upWs;

    outUvTile = vec3(1 - uv.x, uv.y, object.sprites[gl_InstanceIndex].data.x);

    gl_Position = frame.projection * frame.view * object.sprites[gl_InstanceIndex].model * vec4(pos, 1.0);
}
