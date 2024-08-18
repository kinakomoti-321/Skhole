#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_ARB_shading_language_include : require

#include "./payload.glsl"

layout(location = 0) rayPayloadInEXT PayLoadStruct payload;

struct VertexData{
	vec4 position;
	vec4 normal;
	vec2 texcoord0;
	vec2 texcoord1;
	vec4 color;
};

layout(std430,binding = 3) buffer readonly vertexData{
	VertexData vertex[];
}vertex;

hitAttributeEXT vec3 attribs;

void main()
{
	vec3 baryCoords = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	payload.basecolor = baryCoords;
	payload.normal = baryCoords;
    payload.isMiss = false;
}
