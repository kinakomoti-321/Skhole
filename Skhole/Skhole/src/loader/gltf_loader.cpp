#include <loader/gltf_loader.h>
#include <common/filepath.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "tiny_gltf.h"


namespace Skhole {

	//-----------------------------------------------------
	// Array Struct
	//-----------------------------------------------------
	struct IndexBufferArray
	{
		UnqPtr<IntArray<char>> dataChar = nullptr;
		UnqPtr<IntArray<unsigned char>> dataUChar = nullptr;
		UnqPtr<IntArray<short>> dataShort = nullptr;
		UnqPtr<IntArray<unsigned short>> dataUShort = nullptr;
		UnqPtr<IntArray<int>> dataInt = nullptr;
		UnqPtr<IntArray<unsigned int>> dataUInt = nullptr;

		IndexBufferArray(const unsigned char* dataPtr, const size_t count, const size_t byteStride, const int componentType) {
			switch (componentType)
			{
			case TINYGLTF_COMPONENT_TYPE_BYTE:
				dataChar = MakeUnq<IntArray<char>>(dataPtr, count, byteStride);
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
				dataUChar = MakeUnq<IntArray<unsigned char>>(dataPtr, count, byteStride);
				break;
			case TINYGLTF_COMPONENT_TYPE_SHORT:
				dataShort = MakeUnq<IntArray<short>>(dataPtr, count, byteStride);
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
				dataUShort = MakeUnq<IntArray<unsigned short>>(dataPtr, count, byteStride);
				break;
			case TINYGLTF_COMPONENT_TYPE_INT:
				dataInt = MakeUnq<IntArray<int>>(dataPtr, count, byteStride);
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
				dataUChar = MakeUnq<IntArray<unsigned char>>(dataPtr, count, byteStride);
				break;
			default:
				SKHOLE_UNIMPL("Not Compatible Index Format");
				break;
			}
		}

		uint32_t operator[](size_t index) const {
			if (dataChar) return static_cast<uint32_t>((*dataChar)[index]);
			if (dataUChar) return static_cast<uint32_t>((*dataUChar)[index]);
			if (dataShort) return static_cast<uint32_t>((*dataShort)[index]);
			if (dataUShort) return static_cast<uint32_t>((*dataUShort)[index]);
			if (dataInt) return static_cast<uint32_t>((*dataInt)[index]);
			if (dataUInt) return static_cast<uint32_t>((*dataUInt)[index]);
			SKHOLE_UNIMPL("Not Compatible Index Format");
			return 0;
		}
	};

	struct Vector2Array {
		UnqPtr<v2fArray> adapterFloat = nullptr;
		UnqPtr<v2dArray> adapterDouble = nullptr;
		Vector2Array(const unsigned char* dataPtr, const size_t count, const size_t byteStride, const int componentType)
		{
			switch (componentType)
			{
			case TINYGLTF_COMPONENT_TYPE_FLOAT:
				adapterFloat = MakeUnq<v2fArray>(dataPtr, count, byteStride);
				break;
			case TINYGLTF_COMPONENT_TYPE_DOUBLE:
				adapterDouble = MakeUnq<v2dArray>(dataPtr, count, byteStride);
				break;
			default:
				break;
			}
		}

		v2f operator[](size_t index) const {
			if (adapterFloat) return (*adapterFloat)[index];
			if (adapterDouble) {
				v2d f = (*adapterDouble)[index];
				return { (float)f.x, (float)f.y };
			}
			SKHOLE_UNIMPL("Not Compatible Vector2 Format");
			return v2f();
		}
	};

	struct Vector3Array {
		UnqPtr<v3fArray> adapterFloat = nullptr;
		UnqPtr<v3dArray> adapterDouble = nullptr;

		Vector3Array(const unsigned char* dataPtr, const size_t count, const size_t byteStride, const int componentType)
		{
			switch (componentType)
			{
			case TINYGLTF_COMPONENT_TYPE_FLOAT:
				adapterFloat = MakeUnq<v3fArray>(dataPtr, count, byteStride);
				break;
			case TINYGLTF_COMPONENT_TYPE_DOUBLE:
				adapterDouble = MakeUnq<v3dArray>(dataPtr, count, byteStride);
				break;
			default:
				break;
			}
		}

		v3f operator[](size_t index) const {
			if (adapterFloat) return (*adapterFloat)[index];
			if (adapterDouble) {
				v3d f = (*adapterDouble)[index];
				return { (float)f.x, (float)f.y, (float)f.z };
			}
			SKHOLE_UNIMPL("Not Compatible Vector3 Format");
			return v3f();
		}
	};

	struct Vector4Array {
		UnqPtr<v4fArray> adapterFloat = nullptr;
		UnqPtr<v4dArray> adapterDouble = nullptr;

		Vector4Array(const unsigned char* dataPtr, const size_t count, const size_t byteStride, const int componentType)
		{
			switch (componentType)
			{
			case TINYGLTF_COMPONENT_TYPE_FLOAT:
				adapterFloat = MakeUnq<v4fArray>(dataPtr, count, byteStride);
				break;
			case TINYGLTF_COMPONENT_TYPE_DOUBLE:
				adapterDouble = MakeUnq<v4dArray>(dataPtr, count, byteStride);
				break;
			default:
				break;
			}
		}

		v4f operator[](size_t index) const {
			if (adapterFloat) return (*adapterFloat)[index];
			if (adapterDouble) {
				v4d f = (*adapterDouble)[index];
				return { (float)f.x, (float)f.y, (float)f.z, (float)f.w };
			}
			SKHOLE_UNIMPL("Not Compatible Vector4 Format");
			return v4f();
		}
	};

	//-----------------------------------------------------
	// GLTF Loader
	//-----------------------------------------------------
	bool LoadGLTFFile(
		const std::string& filename,
		std::vector<ShrPtr<Object>>& inObjects,
		std::vector<ShrPtr<Geometry>>& inGeometies,
		std::vector<ShrPtr<BasicMaterial>>& inBasicMaterials,
		std::vector<ShrPtr<Texture>>& inTextures
	)
	{
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;


		std::string ext;
		GetFileExtension(filename, ext);

		bool ret;
		if (ext == "glb") {
			ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
		}
		else {
			ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
		}

		if (!warn.empty()) {
			SKHOLE_WARN(warn);
		}

		if (!err.empty()) {
			SKHOLE_ERROR(err);
		}

		if (!ret) {
			SKHOLE_ERROR("Failed to load glTF file");
			return false;
		}

		inObjects.clear();
		inGeometies.clear();
		inBasicMaterials.clear();
		inTextures.clear();

		bool haveMaterial = model.materials.size() > 0;

		//-----------------------------------------------------
		// Load Mesh
		//-----------------------------------------------------
		auto& modelMesh = model.meshes;
		//inGeometies.resize(model.meshes.size());
		for (const auto& mesh : modelMesh) {
			auto geometry = MakeShr<Geometry>();

			std::vector<vec3> position;
			std::vector<vec3> normal;
			std::vector<vec2> texcoord0;
			std::vector<vec2> texcoord1;
			std::vector<vec4> color;

			uint32_t indexOffset = 0;

			for (const auto& prim : mesh.primitives)
			{
				const auto& idxAccessor = model.accessors[prim.indices];
				const auto& idxBufferView = model.bufferViews[idxAccessor.bufferView];
				const auto& idxByteBuffer = model.buffers[idxBufferView.buffer];

				const auto dataPtr = idxByteBuffer.data.data() + idxBufferView.byteOffset + idxAccessor.byteOffset;
				const auto count = idxAccessor.count;
				const auto byteStride = idxAccessor.ByteStride(idxBufferView);


				IndexBufferArray indexArray(dataPtr, count, byteStride, idxAccessor.componentType);
				for (size_t i = 0; i < count; i++)
				{
					auto idx = indexArray[i];
					geometry->m_indices.push_back(idx + indexOffset);
				}


				uint32_t vertexCount = 0;

				for (const auto& attribute : prim.attributes)
				{
					const auto& accessor = model.accessors[attribute.second];
					const auto& bufferView = model.bufferViews[accessor.bufferView];
					const auto& byteBuffer = model.buffers[bufferView.buffer];

					const auto dataPtr = byteBuffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
					const auto count = accessor.count;
					const auto byteStride = accessor.ByteStride(bufferView);

					if (attribute.first == "POSITION") {
						Vector3Array positionArray(dataPtr, count, byteStride, accessor.componentType);
						for (size_t i = 0; i < count; i++)
						{
							auto pos = positionArray[i];
							position.push_back({ pos.x, pos.y, pos.z });
						}

						vertexCount = count;
					}

					if (attribute.first == "NORMAL") {
						Vector3Array normalArray(dataPtr, count, byteStride, accessor.componentType);
						for (size_t i = 0; i < count; i++)
						{
							auto n = normalArray[i];
							normal.push_back({ n.x, n.y, n.z });
						}
					}

					if (attribute.first == "TEXCOORD_0") {
						Vector2Array texcoord0Array(dataPtr, count, byteStride, accessor.componentType);
						for (size_t i = 0; i < count; i++)
						{
							auto t = texcoord0Array[i];
							texcoord0.push_back({ t.x, t.y });
						}
					}

					if (attribute.first == "TEXCOORD_1") {
						Vector2Array texcoord1Array(dataPtr, count, byteStride, accessor.componentType);
						for (size_t i = 0; i < count; i++)
						{
							auto t = texcoord1Array[i];
							texcoord1.push_back({ t.x, t.y });
						}
					}

					if (attribute.first == "COLOR_0") {
						Vector4Array colorArray(dataPtr, count, byteStride, accessor.componentType);
						for (size_t i = 0; i < count; i++)
						{
							auto c = colorArray[i];
							color.push_back({ c.x, c.y, c.z, c.w });
						}
					}
				} // end of attribute loop

				indexOffset += vertexCount;

				//Material Index
				for (int i = 0; i < count / 3; i++) {
					if (haveMaterial)
						geometry->m_materialIndices.push_back(prim.material);
					else
						geometry->m_materialIndices.push_back(0);
				}

			}// end of primitive loop

			auto& geometryVertex = geometry->m_vertices;
			SKHOLE_ASSERT(position.size() > 0);
			for (size_t i = 0; i < position.size(); i++)
			{
				VertexData vertex;
				vertex.position = vec4(position[i], 1.0);
				if (normal.size() >= i + 1) vertex.normal = vec4(normal[i], 0.0);
				else vertex.normal = vec4(0, 1, 0, 0);
				if (texcoord0.size() >= i + 1) {
					vertex.texcoord0[0] = texcoord0[i].x;
					vertex.texcoord0[1] = texcoord0[i].y;
				}
				else {
					vertex.texcoord0[0] = 1.0f;
					vertex.texcoord0[1] = 1.0f;
				}
				if (texcoord1.size() >= i + 1) {
					vertex.texcoord1[0] = texcoord1[i].x;
					vertex.texcoord1[1] = texcoord1[i].y;
				}
				else {
					vertex.texcoord1[0] = 1.0f;
					vertex.texcoord1[1] = 1.0f;
				}
				if (color.size() >= i + 1) vertex.color = color[i];
				else vertex.color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

				geometryVertex.push_back(vertex);
			}

			inGeometies.push_back(geometry);

		}// end of mesh loop

		//-----------------------------------------------------
		// Load Object
		//-----------------------------------------------------
		auto& modelNode = model.nodes;
		for (const auto& node : modelNode)
		{
			//object->objectName = node.name;
			//object->localPosition = vec3(node.translation[0], node.translation[1], node.translation[2]);
			//object->localRotationEular = vec3(node.rotation[0], node.rotation[1], node.rotation[2]); // TODO Quaternion
			//object->localScale = vec3(node.scale[0], node.scale[1], node.scale[2]);


			ShrPtr<Object> object;

			if (node.mesh != -1)
			{
				ShrPtr<Instance> instance = MakeShr<Instance>();
				instance->geometryIndex = node.mesh;

				object = instance;
			}
			else if (node.skin != -1)
			{
				SKHOLE_UNIMPL("Not Comaptible Skin");
			}
			else if (node.camera != -1)
			{
				SKHOLE_UNIMPL("Not Comaptible Camera");
			}
			else if (node.light != -1)
			{
				SKHOLE_UNIMPL("Not Comaptible Light");
			}

			object->objectName = node.name;
			if (node.translation.size() > 0) {
				object->localTranslation = vec3(node.translation[0], node.translation[1], node.translation[2]);
			}
			else {
				object->localTranslation = vec3(0.0f);
			}

			if (node.rotation.size() > 0) {
				object->localQuaternion = Quaternion(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]); // TODO Quaternion
			}
			else {
				object->localQuaternion = Quaternion(0.0, 0.0, 0.0, 1.0f);
			}

			if (node.scale.size() > 0) {
				object->localScale = vec3(node.scale[0], node.scale[1], node.scale[2]);
			}
			else {
				object->localScale = vec3(1.0f);
			}

			inObjects.push_back(object);
		}

		// Create Parent-Child Relationship
		for (int i = 0; i < modelNode.size(); i++)
		{
			auto& node = modelNode[i];
			for (auto childIndex : node.children)
			{
				inObjects[i]->childObject = inObjects[childIndex];
				inObjects[childIndex]->parentObject = inObjects[i];
			}
		}

		//-----------------------------------------------------
		// Load Material
		//-----------------------------------------------------
		auto& materials = model.materials;
		if (haveMaterial) {
			inBasicMaterials.resize(materials.size());
			int index = 0;
			for (auto& mat : materials) {
				auto basicMaterial = MakeShr<BasicMaterial>();
				auto& pbrParam = mat.pbrMetallicRoughness;

				basicMaterial->materialName = mat.name;
				basicMaterial->basecolor = vec4(pbrParam.baseColorFactor[0], pbrParam.baseColorFactor[1], pbrParam.baseColorFactor[2], pbrParam.baseColorFactor[3]);
				basicMaterial->roughness = pbrParam.roughnessFactor;
				basicMaterial->metallic = pbrParam.metallicFactor;

				vec3 emissive = vec3(mat.emissiveFactor[0], mat.emissiveFactor[1], mat.emissiveFactor[2]);
				basicMaterial->emissionColor = vec4(emissive, 1.0);

				//TODO Transmission
				basicMaterial->transmission = 0.0f;
				basicMaterial->ior = 1.0f;

				for (auto& matExtension : mat.extensions) {
					if (matExtension.first == "KHR_materials_clearcoat") {
						const tinygltf::Value::Object& o = matExtension.second.Get<tinygltf::Value::Object>();
						tinygltf::Value::Object::const_iterator it(o.begin());
						tinygltf::Value::Object::const_iterator itEnd(o.end());
						for (; it != itEnd; it++) {
							if (it->first == "clearcoatFactor") {
								basicMaterial->clearcoat = it->second.Get<double>();
							}
						}
					}
					else if (matExtension.first == "KHR_materials_sheen") {
						const tinygltf::Value::Object& o = matExtension.second.Get<tinygltf::Value::Object>();
						tinygltf::Value::Object::const_iterator it(o.begin());
						tinygltf::Value::Object::const_iterator itEnd(o.end());
						for (; it != itEnd; it++) {
							if (it->first == "sheenRoughnessFactor") {
								basicMaterial->sheen = it->second.Get<double>();
							}
						}
					}
					else if (matExtension.first == "KHR_materials_transmission") {
						const tinygltf::Value::Object& o = matExtension.second.Get<tinygltf::Value::Object>();
						tinygltf::Value::Object::const_iterator it(o.begin());
						tinygltf::Value::Object::const_iterator itEnd(o.end());
						for (; it != itEnd; it++) {
							if (it->first == "transmissionFactor") {
								basicMaterial->transmission = it->second.Get<double>();
							}
						}
					}
					else if (matExtension.first == "KHR_materials_ior") {
						const tinygltf::Value::Object& o = matExtension.second.Get<tinygltf::Value::Object>();
						tinygltf::Value::Object::const_iterator it(o.begin());
						tinygltf::Value::Object::const_iterator itEnd(o.end());
						for (; it != itEnd; it++) {
							if (it->first == "ior") {
								basicMaterial->ior = it->second.Get<double>();
							}
						}
					}

					else if (matExtension.first == "KHR_materials_emissive_strength") {
						const tinygltf::Value::Object& o = matExtension.second.Get<tinygltf::Value::Object>();
						tinygltf::Value::Object::const_iterator it(o.begin());
						tinygltf::Value::Object::const_iterator itEnd(o.end());
						for (; it != itEnd; it++) {
							if (it->first == "emissiveStrength") {
								basicMaterial->emissionIntensity = it->second.Get<double>();
							}
						}
					}
				}


				//TODO Texture

				inBasicMaterials[index] = basicMaterial;
				index++;
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

		auto& modelAnim = model.animations;
		auto& modelMaterial = model.materials;

		//-----------------------------------------------------
		// Animation
		//-----------------------------------------------------
		{
			for (int i = 0; i < modelAnim.size(); i++)
			{
				auto& anim = modelAnim[i];
				auto& object = inObjects[i];

				for (int i = 0; i < anim.channels.size(); i++) {
					auto& sampler = anim.samplers[i];
					auto& channel = anim.channels[i];
					auto& animKeyAccessor = model.accessors[sampler.input];
					auto& animDataAccessor = model.accessors[sampler.output];

					auto& animKeyBufferView = model.bufferViews[animKeyAccessor.bufferView];
					auto& keyBuffer = model.buffers[animKeyBufferView.buffer];
					auto keyPtr = keyBuffer.data.data() + animKeyBufferView.byteOffset + animKeyAccessor.byteOffset;
					auto keyByteStride = animKeyAccessor.ByteStride(animKeyBufferView);
					auto keyCount = animKeyAccessor.count;

					FloatArray keyArray(keyPtr, keyByteStride, keyCount);

					auto& animDataBufferView = model.bufferViews[animDataAccessor.bufferView];
					auto& dataBuffer = model.buffers[animDataBufferView.buffer];
					auto dataPtr = dataBuffer.data.data() + animDataBufferView.byteOffset + animDataAccessor.byteOffset;
					auto dataByteStride = animDataAccessor.ByteStride(animDataBufferView);
					auto dataCount = animDataAccessor.count;

					if (channel.target_path == "translation") {
						v3fArray data(dataPtr, dataCount, dataByteStride);
						for (size_t i = 0; i < dataCount; i++)
						{
							v3f translation = data[i];
							float key = keyArray[i];
							KeyFrame<vec3> keyFrame;
							keyFrame.value = vec3(translation.x, translation.y, translation.z);
							keyFrame.frame = key;
							object->translationAnimation.AppendKey(keyFrame);
						}
					}
					else if (channel.target_path == "rotation") {
						v4fArray data(dataPtr, dataCount, dataByteStride);
						for (size_t i = 0; i < dataCount; i++)
						{
							v4f rotation = data[i];
							float key = keyArray[i];
							KeyFrame<Quaternion> keyFrame;
							keyFrame.value = Quaternion(rotation.x, rotation.y, rotation.z, rotation.w);
							keyFrame.frame = key;
							object->rotationAnimation.AppendKey(keyFrame);
						}
					}
					else if (channel.target_path == "scale") {
						v3fArray data(dataPtr, dataCount, dataByteStride);
						for (size_t i = 0; i < dataCount; i++)
						{
							v3f scale = data[i];
							float key = keyArray[i];
							KeyFrame<vec3> keyFrame;
							keyFrame.value = vec3(scale.x, scale.y, scale.z);
							keyFrame.frame = key;
							object->scaleAnimation.AppendKey(keyFrame);
						}
					}
				}
				

			}
		}


		SKHOLE_LOG("Loaded GLTF file");

		return true;
	}
}