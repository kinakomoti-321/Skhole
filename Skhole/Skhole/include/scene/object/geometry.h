#pragma once
#include <include.h>

using namespace VectorLikeGLSL;
namespace Skhole {

	struct VertexData {
		vec3 position;
		vec3 normal;
		vec2 texcoord0;
		vec2 texcoord1;
		vec3 color;
	};

	class Geometry {
	public:
		Geometry() {};
		Geometry(const Geometry& geom) {}
		
		~Geometry() {};

		std::vector<VertexData> m_vertices;

		std::vector<uint32_t> m_indices;
		std::vector<uint32_t> m_materialIndices;
	};

}
