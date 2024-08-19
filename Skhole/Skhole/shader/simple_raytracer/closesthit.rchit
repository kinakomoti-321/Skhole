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

struct GeometryData{
	int vertexOffset;
	int indexOffset;
};

struct InstanceData{
	uint geometryIndex;
	vec4 transform0;
	vec4 transform1;
	vec4 transform2;

	vec4 normalTransform0;
	vec4 normalTransform1;
	vec4 normalTransform2;
};

layout(std430,binding = 3) buffer readonly vertexData{
	VertexData vertex[];
};

layout(binding=4) buffer readonly indexData{
	uint index[];
};

layout(binding=5) buffer readonly geometryData{
	GeometryData geometry[];
};

layout(binding=6) buffer readonly instanceData{
	InstanceData instance[];
};

hitAttributeEXT vec3 attribs;

void main()
{
	vec3 baryCoords = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	payload.basecolor = baryCoords;
	payload.normal = baryCoords;
    payload.isMiss = false;
}
