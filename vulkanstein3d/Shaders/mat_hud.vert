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

layout(push_constant) uniform uPushConstant {	
    mat4 vp;
    vec2 scale;
    vec2 translate;
	int textureIndex;
} pc;

layout(location = 0) out struct {    
    vec3 uvTile;
} Out;

void main()
{
	vec2 uv = uvs[gl_VertexIndex];
	vec2 pos = vec2(0.0f, 0.0f);
	pos.x = pc.scale.x * (0.5 - uv.x);
    pos.y = pc.scale.y * (0.5 - uv.y);
	pos += pc.translate;

    Out.uvTile = vec3(vec2(uv.x, 1 - uv.y), float(pc.textureIndex));
    gl_Position = pc.vp * vec4(pos, 0, 1);
}