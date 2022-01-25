#version 450

layout(push_constant) uniform ObjectPushConstants
{
    mat4 mvp;
    float tileIndex;
    float padding0;
    float padding1;
    float padding2;    
} consts;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inUvTile;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outUvTile;

void main() {    
    outNormal = inNormal;
    outUvTile = vec3(inUvTile.xy, consts.tileIndex);
    gl_Position = consts.mvp * vec4(inPos, 1.0);
}