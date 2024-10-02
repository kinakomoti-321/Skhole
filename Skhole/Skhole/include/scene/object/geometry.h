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

		bool useConnectIndex = false;

		std::vector<uint32_t> m_connectPrimId;
	};


	// Connect Prim Id
	//   v2
	//   |  \
	//   |   \
	//   |    \   1
	// 2 |     \
	//   |      \
	//   |       \
	//  v0 ------ v1
	//       0

	inline std::pair<uint32_t, uint32_t> GetEdge(uint32_t v0, uint32_t v1)
	{
		if (v0 < v1)
			return std::make_pair(v0, v1);
		else
			return std::make_pair(v1, v0);
	}

	inline uint32_t GetPrimIdWithSharedEdge(const uint32_t myselfPrimId, const std::vector<uint32_t>& edgePrimId)
	{
		if (edgePrimId.size() == 2) {
			if (edgePrimId[0] == myselfPrimId) {
				return edgePrimId[1];
			}
			else {
				return edgePrimId[0];
			}
		}
		else {
			return -1;
		}
	}

	inline void CreateConnectPrimId(ShrPtr<Geometry>& geom)
	{
		auto& indices = geom->m_indices;
		auto& connectIndices = geom->m_connectPrimId;
		std::map<std::pair<uint32_t, uint32_t>, std::vector<uint32_t>> edgeMap;

		for (int i = 0; i < indices.size() / 3; i++)
		{
			uint32_t v0 = indices[i * 3 + 0];
			uint32_t v1 = indices[i * 3 + 1];
			uint32_t v2 = indices[i * 3 + 2];

			edgeMap[GetEdge(v0, v1)].push_back(i);
			edgeMap[GetEdge(v1, v2)].push_back(i);
			edgeMap[GetEdge(v2, v0)].push_back(i);
		}

		connectIndices.resize(indices.size());
		for (int i = 0; i < indices.size() / 3; i++)
		{
			// Serch Edge
			uint32_t v0 = indices[i * 3 + 0];
			uint32_t v1 = indices[i * 3 + 1];
			uint32_t v2 = indices[i * 3 + 2];

			connectIndices[i * 3 + 0] = GetPrimIdWithSharedEdge(i, edgeMap[GetEdge(v0, v1)]);
			connectIndices[i * 3 + 1] = GetPrimIdWithSharedEdge(i, edgeMap[GetEdge(v1, v2)]);
			connectIndices[i * 3 + 2] = GetPrimIdWithSharedEdge(i, edgeMap[GetEdge(v2, v0)]);
		}

		geom->useConnectIndex = true;

		SKHOLE_LOG("Created Connecte Prim Id");
	}
}
