#pragma once
#include <include.h>
#include <scene/object/geometry.h>

namespace Skhole {

	inline Geometry BoxGeometry() {
		Geometry box;
		box.m_vertices.resize(8);
		box.m_indices.resize(36);
		box.m_materialIndices.resize(12);

		box.m_vertices[0].position = vec4(-1.0f, -1.0f, 1.0f, 1.0f);
		box.m_vertices[1].position = vec4(1.0f, -1.0f, 1.0f, 1.0f);
		box.m_vertices[2].position = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		box.m_vertices[3].position = vec4(-1.0f, 1.0f, 1.0f, 1.0f);
		box.m_vertices[4].position = vec4(-1.0f, -1.0f, -1.0f, 1.0f);
		box.m_vertices[5].position = vec4(1.0f, -1.0f, -1.0f, 1.0f);
		box.m_vertices[6].position = vec4(1.0f, 1.0f, -1.0f, 1.0f);
		box.m_vertices[7].position = vec4(-1.0f, 1.0f, -1.0f, 1.0f);

		box.m_vertices[0].normal = vec4(0.0f, 0.0f, 1.0f, 0.0f);
		box.m_vertices[1].normal = vec4(0.0f, 0.0f, 1.0f, 0.0f);
		box.m_vertices[2].normal = vec4(0.0f, 0.0f, 1.0f, 0.0f);
		box.m_vertices[3].normal = vec4(0.0f, 0.0f, 1.0f, 0.0f);
		box.m_vertices[4].normal = vec4(0.0f, 0.0f, -1.0f, 0.0f);
		box.m_vertices[5].normal = vec4(0.0f, 0.0f, -1.0f, 0.0f);
		box.m_vertices[6].normal = vec4(0.0f, 0.0f, -1.0f, 0.0f);
		box.m_vertices[7].normal = vec4(0.0f, 0.0f, -1.0f, 0.0f);

		box.m_vertices[0].texcoord0[0] = 0.0f;
		box.m_vertices[0].texcoord0[1] = 0.0f;
		box.m_vertices[1].texcoord0[0] = 1.0f;
		box.m_vertices[1].texcoord0[1] = 0.0f;
		box.m_vertices[2].texcoord0[0] = 1.0f;
		box.m_vertices[2].texcoord0[1] = 0.0f;
		box.m_vertices[3].texcoord0[0] = 1.0f;
		box.m_vertices[3].texcoord0[1] = 0.0f;
		box.m_vertices[4].texcoord0[0] = 1.0f;
		box.m_vertices[4].texcoord0[1] = 1.0f;
		box.m_vertices[5].texcoord0[0] = 0.0f;
		box.m_vertices[5].texcoord0[1] = 1.0f;
		box.m_vertices[6].texcoord0[0] = 0.0f;
		box.m_vertices[6].texcoord0[1] = 1.0f;
		box.m_vertices[7].texcoord0[0] = 1.0f;
		box.m_vertices[7].texcoord0[1] = 1.0f;

		box.m_vertices[0].texcoord1[0] = 0.0f;
		box.m_vertices[0].texcoord1[1] = 0.0f;
		box.m_vertices[1].texcoord1[0] = 1.0f;
		box.m_vertices[1].texcoord1[1] = 0.0f;
		box.m_vertices[2].texcoord1[0] = 1.0f;
		box.m_vertices[2].texcoord1[1] = 0.0f;
		box.m_vertices[3].texcoord1[0] = 1.0f;
		box.m_vertices[3].texcoord1[1] = 0.0f;
		box.m_vertices[4].texcoord1[0] = 1.0f;
		box.m_vertices[4].texcoord1[1] = 1.0f;
		box.m_vertices[5].texcoord1[0] = 0.0f;
		box.m_vertices[5].texcoord1[1] = 1.0f;
		box.m_vertices[6].texcoord1[0] = 0.0f;
		box.m_vertices[6].texcoord1[1] = 1.0f;
		box.m_vertices[7].texcoord1[0] = 1.0f;
		box.m_vertices[7].texcoord1[1] = 1.0f;


		box.m_vertices[0].color = vec4(0.0f, 0.0f, 1.0f, 0.0f);
		box.m_vertices[1].color = vec4(1.0f, 0.0f, 1.0f, 0.0f);
		box.m_vertices[2].color = vec4(1.0f, 1.0f, 1.0f, 0.0f);
		box.m_vertices[3].color = vec4(0.0f, 1.0f, 1.0f, 0.0f);
		box.m_vertices[4].color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		box.m_vertices[5].color = vec4(1.0f, 0.0f, 0.0f, 0.0f);
		box.m_vertices[6].color = vec4(1.0f, 1.0f, 0.0f, 0.0f);
		box.m_vertices[7].color = vec4(0.0f, 1.0f, 0.0f, 0.0f);

		box.m_indices = {
			0, 1, 2, 0, 2, 3,
			1, 5, 6, 1, 6, 2,
			5, 4, 7, 5, 7, 6,
			4, 0, 3, 4, 3, 7,
			3, 2, 6, 3, 6, 7,
			4, 5, 1, 4, 1, 0
		};

		box.m_materialIndices = {
			0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0
		};

		return box;
	}

	inline Geometry PlaneGeometry(const vec3& wVector,const vec3& hVector,const vec3& origin, uint32_t materialID) {
		Geometry plane;
		plane.m_vertices.resize(4);
		plane.m_indices.resize(6);
		plane.m_materialIndices.resize(2);

		vec3 normal = normalize(cross(wVector, hVector));

		plane.m_vertices[0].position = vec4(origin, 1.0f);
		plane.m_vertices[1].position = vec4(origin + wVector, 1.0f);
		plane.m_vertices[2].position = vec4(origin + hVector, 1.0f);
		plane.m_vertices[3].position = vec4(origin + wVector + hVector, 1.0f);
		
		plane.m_vertices[0].normal = vec4(normal, 0.0f);
		plane.m_vertices[1].normal = vec4(normal, 0.0f);
		plane.m_vertices[2].normal = vec4(normal, 0.0f);
		plane.m_vertices[3].normal = vec4(normal, 0.0f);

		plane.m_vertices[0].texcoord0[0] = 0.0f;
		plane.m_vertices[0].texcoord0[1] = 0.0f;
		plane.m_vertices[1].texcoord0[0] = 1.0f;
		plane.m_vertices[1].texcoord0[1] = 0.0f;
		plane.m_vertices[2].texcoord0[0] = 0.0f;
		plane.m_vertices[2].texcoord0[1] = 1.0f;
		plane.m_vertices[3].texcoord0[0] = 1.0f;
		plane.m_vertices[3].texcoord0[1] = 1.0f;

		plane.m_vertices[0].texcoord1[0] = 0.0f;
		plane.m_vertices[0].texcoord1[1] = 0.0f;
		plane.m_vertices[1].texcoord1[0] = 1.0f;
		plane.m_vertices[1].texcoord1[1] = 0.0f;
		plane.m_vertices[2].texcoord1[0] = 0.0f;
		plane.m_vertices[2].texcoord1[1] = 1.0f;
		plane.m_vertices[3].texcoord1[0] = 1.0f;
		plane.m_vertices[3].texcoord1[1] = 1.0f;

		plane.m_vertices[0].color = vec4(0.0f, 0.0f, 1.0f, 0.0f);
		plane.m_vertices[1].color = vec4(1.0f, 0.0f, 1.0f, 0.0f);
		plane.m_vertices[2].color = vec4(1.0f, 1.0f, 1.0f, 0.0f);	
		plane.m_vertices[3].color = vec4(0.0f, 1.0f, 1.0f, 0.0f);

		plane.m_indices[0] = 0;
		plane.m_indices[1] = 1;
		plane.m_indices[2] = 2;
		plane.m_indices[3] = 2;
		plane.m_indices[4] = 1;
		plane.m_indices[5] = 3;

		plane.m_materialIndices[0] = materialID;
		plane.m_materialIndices[1] = materialID;

		return plane;
	}
}
