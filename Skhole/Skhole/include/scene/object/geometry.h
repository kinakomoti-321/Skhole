#pragma once
#include <include.h>

using namespace VectorLikeGLSL;
namespace Skhole {

	struct VertexData {
		vec4 position;
		vec4 normal;
		float texcoord0[2]; // vec2 is 16bytes, Umm...
		float texcoord1[2];
		vec4 color;
	};

	class Geometry {
	public:
		Geometry() {};
		Geometry(const Geometry& geom) {}
		
		~Geometry() {};

		std::vector<VertexData> m_vertices;

		std::vector<uint32_t> m_indices;
		std::vector<uint32_t> m_materialIndices;

		bool useAnimation = false;
	};

}
