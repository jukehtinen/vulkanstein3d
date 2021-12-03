#version 450

//layout(set = 0, binding = 1) readonly buffer Objects {
//	mat4 mvps[];
//} objs;

layout(push_constant) uniform FrameConstants
{
    float time;
    float mousexNormalized;
    float mouseyNormalized;
    float padding;
    mat4 mvp;
} consts;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inUvTile;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outUvTile;

void main() {    
    outNormal = inNormal;
    outUvTile = inUvTile;
    gl_Position = consts.mvp * vec4(inPos, 1.0);
}