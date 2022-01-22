#version 450

layout(set = 0, binding = 0) uniform sampler2DArray textures;

layout(location = 0) in struct {
    vec3 uvTile;
} In;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(textures, In.uvTile.xyz);
    if (outColor.a < 0.9) discard;
}