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

layout(std430, binding = 3) buffer readonly vertexData{
	VertexData vertex[];
};

layout(std430, binding=4) buffer readonly indexData{
	uint index[];
};

layout(std430, binding=5) buffer readonly geometryData{
	GeometryData geometry[];
};

layout(std430, binding=6) buffer readonly instanceData{
	InstanceData instance[];
};

hitAttributeEXT vec3 attribs;

void main()
{
	uint instanceID = gl_InstanceID;
	InstanceData inst = instance[instanceID];
	GeometryData geom = geometry[inst.geometryIndex];

	uint primID = gl_PrimitiveID;
	uint index0 = index[geom.indexOffset + primID * 3 + 0];
	uint index1 = index[geom.indexOffset + primID * 3 + 1];
	uint index2 = index[geom.indexOffset + primID * 3 + 2];

	VertexData v0 = vertex[geom.vertexOffset + index0];
	VertexData v1 = vertex[geom.vertexOffset + index1];
	VertexData v2 = vertex[geom.vertexOffset + index2];
		
	vec3 baryCoords = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	vec4 position = (1.0 - attribs.x - attribs.y) * v0.position + attribs.x * v1.position + attribs.y * v2.position;
	vec4 normal = (1.0 - attribs.x - attribs.y) * v0.normal + attribs.x * v1.normal + attribs.y * v2.normal;

//	payload.basecolor = (v0.normal.xyz + vec3(1.0)) * 0.5;
	payload.basecolor = vec3(float(inst.geometryIndex) / 6.0); 
//	payload.basecolor =vec3(instanceID / 6.0);
	payload.normal = baryCoords;
    payload.isMiss = false;
}
