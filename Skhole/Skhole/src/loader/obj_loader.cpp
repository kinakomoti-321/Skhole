
#include <loader/obj_loader.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace Skhole {
	bool LoadObjFile(
		const std::string& filename,
		std::vector<ShrPtr<Object>>& inObjects,
		std::vector<ShrPtr<Geometry>>& inGeometies,
		std::vector<ShrPtr<BasicMaterial>>& inBasicMaterials,
		std::vector<ShrPtr<Texture>>& inTextures
	) {

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string warn;
		std::string err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str())) {
			std::cerr << warn << std::endl;
			std::cerr << err << std::endl;
			return false;
		}

		inObjects.clear();
		inGeometies.clear();
		inBasicMaterials.clear();
		inTextures.clear();

		inObjects.resize(shapes.size());
		inGeometies.resize(shapes.size());

		bool haveMaterial = materials.size() > 0;

		for (size_t s = 0; s < shapes.size(); s++) {
			size_t index_offset = 0;
			auto& shape = shapes[s];
			auto geometry = MakeShr<Geometry>();

			// Mesh
			for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
				// Face
				int fv = shape.mesh.num_face_vertices[f];
				for (size_t v = 0; v < fv; v++) {
					// Vertex
					tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
					//Vertex vertex;
					VertexData vertex;

					vertex.position = vec4(
						attrib.vertices[3 * idx.vertex_index + 0],
						attrib.vertices[3 * idx.vertex_index + 1],
						attrib.vertices[3 * idx.vertex_index + 2],
						1.0f
					);

					if (idx.normal_index >= 0) {
						vertex.normal = vec4(
							attrib.normals[3 * idx.normal_index + 0],
							attrib.normals[3 * idx.normal_index + 1],
							attrib.normals[3 * idx.normal_index + 2],
							0.0f
						);
					}
					else {
						vertex.normal = vec4(
							0,
							1,
							0,
							0.0f
						);
					}

					if (idx.texcoord_index >= 0) {
						vertex.texcoord0[0] = attrib.texcoords[2 * idx.texcoord_index + 0];
						vertex.texcoord0[1] = attrib.texcoords[2 * idx.texcoord_index + 1];

						vertex.texcoord1[0] = attrib.texcoords[2 * idx.texcoord_index + 0];
						vertex.texcoord1[1] = attrib.texcoords[2 * idx.texcoord_index + 1];
					}
					else {
						vertex.texcoord0[0] = 1.0f;
						vertex.texcoord0[1] = 1.0f;

						vertex.texcoord1[0] = 1.0f;
						vertex.texcoord1[1] = 1.0f;
					}


					if (attrib.colors.size() > 0) {
						vertex.color = vec4(
							attrib.colors[3 * idx.vertex_index + 0],
							attrib.colors[3 * idx.vertex_index + 1],
							attrib.colors[3 * idx.vertex_index + 2],
							1.0f
						);
					}
					else {
						vertex.color = vec4(
							1.0f,
							1.0f,
							1.0f,
							1.0f
						);
					}

					geometry->m_vertices.push_back(vertex);
					geometry->m_indices.push_back(geometry->m_indices.size());
				}

				index_offset += fv;

				// Material Index
				if (haveMaterial) {
					geometry->m_materialIndices.push_back(shape.mesh.material_ids[f]);
				}
				else {
					geometry->m_materialIndices.push_back(0);
				}

			}
			inGeometies[s] = geometry;

			ShrPtr<Instance> instance = std::make_shared<Instance>();
			instance->objectName = shape.name;
			instance->geometryIndex = s;
			instance->localTranslation = vec3(0.0f);
			instance->localQuaternion = Quaternion();
			instance->localScale = vec3(1.0f);

			inObjects[s] = instance;
		}


		// Material
		if (haveMaterial) {
			inBasicMaterials.resize(materials.size());

			for (size_t m = 0; m < materials.size(); m++) {
				auto& material = materials[m];
				//auto& basicMaterial = inBasicMaterials[m];
				auto basicMaterial = MakeShr<BasicMaterial>();

				basicMaterial->materialName = material.name;
				basicMaterial->basecolor = vec4(material.diffuse[0], material.diffuse[1], material.diffuse[2], 1.0f);
				basicMaterial->metallic = material.metallic;
				basicMaterial->roughness = material.roughness;

				vec3 emission = vec3(material.emission[0], material.emission[1], material.emission[2]);
				float emissionNormalizeFactior = length(emission);

				basicMaterial->emissionIntensity = emissionNormalizeFactior;
				if (emissionNormalizeFactior > 0.0f) {
					basicMaterial->emissionColor = vec4(emission / emissionNormalizeFactior, 1.0f);
				}

				basicMaterial->transmission = material.transmittance[0]; // ?
				basicMaterial->ior = material.ior;

				// TODO Texture

				inBasicMaterials[m] = basicMaterial;
			}
		}
		else {
			inBasicMaterials.resize(1);
			auto basicMaterial = MakeShr<BasicMaterial>();
			basicMaterial->materialName = "DefaultMaterial";
			basicMaterial->basecolor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			basicMaterial->metallic = 0.0f;
			basicMaterial->roughness = 0.5f;
			basicMaterial->emissionIntensity = 0.0f;
			basicMaterial->emissionColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
			basicMaterial->transmission = 0.0f;
			basicMaterial->ior = 1.0f;

			inBasicMaterials[0] = basicMaterial;
		}

		return true;
	};
}
