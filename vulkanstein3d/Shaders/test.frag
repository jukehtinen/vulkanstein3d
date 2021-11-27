#version 450

layout(set = 0, binding = 0) uniform sampler2D textures[];

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform FrameConstants
{
    float time;
    float mousexNormalized;
    float mouseyNormalized;
    float padding;
} consts;

void main()  
{
    float dist = distance(inUV, vec2(consts.mousexNormalized, consts.mouseyNormalized));
    
    outColor = mix(vec4(texture(textures[0], inUV)), vec4(0.0, 0.0, 0.0, 1.0),
        clamp(sin(dist * 10.0 - consts.time), 0.0, 1.0));
}
