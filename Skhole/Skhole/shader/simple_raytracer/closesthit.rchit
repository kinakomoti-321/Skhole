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
	uint padding1;
	uint padding2;
	uint padding3;

	vec4 transform0;
	vec4 transform1;
	vec4 transform2;

	vec4 normalTransform0;
	vec4 normalTransform1;
	vec4 normalTransform2;
};

struct Material{
	vec4 baseColor;

	float roughness;
	float metallic;
	float emissionPower;

	float padding;
	vec4 emissionColor;
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

layout(std430,binding=7) buffer readonly materialData{
	Material materials[];
};

layout(std430, binding=8) buffer readonly matIndexData{
	uint matIndex[];
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

	vec4 normal = (1.0 - attribs.x - attribs.y) * v0.normal + attribs.x * v1.normal + attribs.y * v2.normal;
	normal.w = 0.0;

	mat4 normalTransform = mat4(
	inst.normalTransform0.x, inst.normalTransform1.x, inst.normalTransform2.x, 0.0,
	inst.normalTransform0.y, inst.normalTransform1.y, inst.normalTransform2.y, 0.0,
	inst.normalTransform0.z, inst.normalTransform1.z, inst.normalTransform2.z, 0.0,
	inst.normalTransform0.w, inst.normalTransform1.w, inst.normalTransform2.w, 1.0
	);

	normal = normalTransform * normal;
	
	uint primIdOffset = geom.indexOffset / 3;
	uint materialIndex = matIndex[primIdOffset + primID];
	Material mat = materials[materialIndex];

	payload.basecolor = mat.baseColor.xyz;
	payload.t = gl_HitTEXT;
	payload.position = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	payload.normal = normalize(normal.xyz);
    payload.isMiss = false;

	
	payload.isLight = mat.emissionPower > 0.0;
	payload.emission = mat.emissionColor.xyz * mat.emissionPower;
}
