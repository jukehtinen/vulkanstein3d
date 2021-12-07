#version 450

layout(set = 0, binding = 0) uniform sampler2DArray textures;

layout(location = 0) in vec3 inUvTile;

layout(location = 0) out vec4 outColor;

void main(void)
{
    outColor = texture(textures, inUvTile);
    if (outColor.a < 0.9) discard;
}