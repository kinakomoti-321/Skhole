#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_ARB_shading_language_include : require

#include "./payload.glsl"

layout(location = 0) rayPayloadInEXT PayLoadStruct payLoad;

void main()
{
    payLoad.basecolor = vec3(1.0);
    payLoad.normal = vec3(0.0,1.0,0.0);
    payLoad.isMiss = true;
}