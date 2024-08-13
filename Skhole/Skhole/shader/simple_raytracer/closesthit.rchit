#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_ARB_shading_language_include : require

#include "./payload.glsl"

layout(location = 0) rayPayloadInEXT PayLoadStruct payload;
hitAttributeEXT vec3 attribs;

void main()
{
	vec3 baryCoords = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	payload.basecolor = vec3(0.0,1.0,0.0);
	payload.normal = baryCoords;
    payload.isMiss = false;
}
