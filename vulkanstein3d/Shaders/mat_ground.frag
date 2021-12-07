#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inUvTile;

layout(location = 0) out vec4 outColor;

void main() {    
    outColor = vec4(inUvTile, 1.0);
    //outColor = vec4(0.44, 0.44, 0.44, 1.0);
}