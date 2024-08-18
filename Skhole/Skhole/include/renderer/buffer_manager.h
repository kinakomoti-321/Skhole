#pragma once
#include <include.h>
#include <scene/scene.h>
#include <vulkan_helpler/vk_buffer.h>

namespace Skhole {
	class SceneBufferaManager {
	public:
		SceneBufferaManager() {};
		~SceneBufferaManager() {};

		struct GeometryData {
			uint32_t indexOffset;
		};

		struct GeometryBufferData {
			uint32_t vertexOffsetByte;
			uint32_t indexOffsetByte;

			uint32_t numVert;
			uint32_t numIndex;
		};

		struct InstanceData {
			uint32_t geometryIndex;
			//vec4 transform[3];
			vk::TransformMatrixKHR transform;
			vk::TransformMatrixKHR invTransform;
		};

		void SetScene(ShrPtr<Scene> in_scene) {
			scene = in_scene;
		}

		void InitGeometryBuffer(vk::PhysicalDevice physicalDevice, vk::Device device) {

			auto geometries = scene->m_geometies;

			uint32_t vertexCount = 0;
			uint32_t indexCount = 0;

			for (auto& geometry : geometries) {
				auto& vertices = geometry->m_vertices;
				auto& indices = geometry->m_indices;

				vertexCount += vertices.size();
				indexCount += indices.size();
			}

			vk::BufferUsageFlags bufferUsage{
				vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
				//vk::BufferUsageFlagBits::eStorageBuffer |
				vk::BufferUsageFlagBits::eShaderDeviceAddress
			};

			vk::MemoryPropertyFlags memoryProperty{
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			};

			vertexBuffer.init(
				physicalDevice, device,
				vertexCount * sizeof(VertexData),
				bufferUsage, memoryProperty
			);

			indexBuffer.init(
				physicalDevice, device,
				indexCount * sizeof(uint32_t),
				bufferUsage, memoryProperty
			);

			uint32_t vertOffsetByte = 0;
			uint32_t indexOffsetByte = 0;

			uint32_t indexOffset = 0;
			uint32_t vertexOffset = 0;

			for (auto& geometry : geometries) {
				auto& vertices = geometry->m_vertices;
				auto& indices = geometry->m_indices;

				void* vertexMap = vertexBuffer.Map(device, vertOffsetByte, vertices.size() * sizeof(VertexData));
				void* indexMap = indexBuffer.Map(device, indexOffsetByte, indices.size() * sizeof(uint32_t));

				memcpy(vertexMap, vertices.data(), vertices.size() * sizeof(VertexData));
				memcpy(indexMap, indices.data(), indices.size() * sizeof(uint32_t));

				vertexBuffer.Ummap(device);
				indexBuffer.Ummap(device);

				GeometryData geomData;
				geomData.indexOffset = indexOffset;
				geometryData.push_back(geomData);

				GeometryBufferData geomOffset;
				geomOffset.vertexOffsetByte = vertOffsetByte;
				geomOffset.indexOffsetByte = indexOffsetByte;
				geomOffset.numVert = vertices.size();
				geomOffset.numIndex = indices.size();
				geometryOffset.push_back(geomOffset);

				vertOffsetByte += vertices.size() * sizeof(VertexData);
				indexOffsetByte += indices.size() * sizeof(uint32_t);

				indexOffset += indices.size();
			}
		}

		void InitInstanceBuffer(vk::PhysicalDevice physicalDevice, vk::Device device) {
			auto& objects = scene->m_objects;

			for (auto& object : objects) {
				if (ObjectType::INSTANCE == object->GetObjectType()) {
					auto instance = std::static_pointer_cast<Instance>(object);

					InstanceData instData;
					instData.geometryIndex = instance->geometryIndex.value();

					//instData.transform[0] = std::array<float, 4>{ 1.0f, 0.0f, 0.0f, 0.0f };
					//instData.transform[1] = std::array<float, 4>{ 0.0f, 1.0f, 0.0f, 0.0f };
					//instData.transform[2] = std::array<float, 4>{ 0.0f, 0.0f, 1.0f, 0.0f };

					instData.transform = std::array{
						std::array{1.0f, 0.0f, 0.0f, 0.0f},
						std::array{0.0f, 1.0f, 0.0f, 0.0f},
						std::array{0.0f, 0.0f, 1.0f, 0.0f}
					};

					instData.invTransform = std::array{
						std::array{1.0f, 0.0f, 0.0f, 0.0f},
						std::array{0.0f, 1.0f, 0.0f, 0.0f},
						std::array{0.0f, 0.0f, 1.0f, 0.0f}
					};

					instanceData.push_back(instData);
				}
			}

			vk::BufferUsageFlags bufferUsage{
				vk::BufferUsageFlagBits::eStorageBuffer |
				vk::BufferUsageFlagBits::eShaderDeviceAddress
			};

			vk::MemoryPropertyFlags memoryProperty{
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			};

			instanceBuffer.init(
				physicalDevice, device,
				instanceData.size() * sizeof(InstanceData),
				bufferUsage, memoryProperty
			);

			void* instanceMap = instanceBuffer.Map(device, 0, instanceData.size() * sizeof(InstanceData));
			memcpy(instanceMap, instanceData.data(), instanceData.size() * sizeof(InstanceData));
			instanceBuffer.Ummap(device);
		}

		void FrameUpdateInstance(vk::Device device, float frame) {
			auto& objects = scene->m_objects;
			uint32_t numInstance = instanceData.size();
			instanceData.clear();
			instanceData.reserve(numInstance);

			for (auto& object : objects) {
				if (ObjectType::INSTANCE == object->GetObjectType()) {
					auto instance = std::static_pointer_cast<Instance>(object);

					InstanceData instData;
					instData.geometryIndex = instance->geometryIndex.value();

					mat4 transform = instance->GetWorldTransformMatrix(frame);

					instData.transform = std::array{
						std::array{transform[0][0], transform[0][1], transform[0][2], transform[0][3]},
						std::array{transform[1][0], transform[1][1], transform[1][2], transform[1][3]},
						std::array{transform[2][0], transform[2][1], transform[2][2], transform[2][3]}
					};

					instData.invTransform = std::array{
						std::array{1,0,0,0},
						std::array{0,1,0,0},
						std::array{0,0,1,0}
					};

					instanceData.push_back(instData);
				}
			}

			void* instanceMap = instanceBuffer.Map(device, 0, instanceData.size() * sizeof(InstanceData));
			memcpy(instanceMap, instanceData.data(), instanceData.size() * sizeof(InstanceData));
			instanceBuffer.Ummap(device);
		}

		void Release(vk::Device device) {
			vertexBuffer.Release(device);
			indexBuffer.Release(device);

			geometryBuffer.Release(device);
			instanceBuffer.Release(device);
		}

		Buffer vertexBuffer;
		Buffer indexBuffer;

		std::vector<GeometryData> geometryData;
		std::vector<GeometryBufferData> geometryOffset;

		std::vector<InstanceData> instanceData;

		Buffer geometryBuffer;
		Buffer instanceBuffer;

		ShrPtr<Scene> scene = nullptr;
	};

	class ASManager {
	public:
		ASManager() {};
		~ASManager() {};

		void BuildBLAS(ShrPtr<SceneBufferaManager>& bufferManager, vk::PhysicalDevice physicalDevice, vk::Device device, vk::CommandPool commandPool, vk::Queue queue) {

			auto& geomOffset = bufferManager->geometryOffset;

			for (auto& geom : geomOffset) {
				vk::AccelerationStructureGeometryTrianglesDataKHR triangles{};
				triangles.setVertexFormat(vk::Format::eR32G32B32Sfloat);
				triangles.setVertexData(bufferManager->geometryBuffer.address + geom.vertexOffsetByte);
				triangles.setVertexStride(sizeof(VertexData));
				triangles.setMaxVertex(geom.numVert);
				triangles.setIndexType(vk::IndexType::eUint32);
				triangles.setIndexData(bufferManager->indexBuffer.address + geom.indexOffsetByte);

				vk::AccelerationStructureGeometryKHR geometry{};
				geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
				geometry.setGeometry({ triangles });
				geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

				uint32_t primitiveCount = geom.numIndex / 3;
				AccelStruct blas;
				blas.init(physicalDevice, device, commandPool, queue,
					vk::AccelerationStructureTypeKHR::eBottomLevel,
					geometry, primitiveCount);

				BLASes.push_back(blas);
			}
		}

		void UpdateBLAS() {
			SKHOLE_UNIMPL("Update BLAS");
		}

		void BuildTLAS(const ShrPtr<SceneBufferaManager>& bufferManager, vk::PhysicalDevice physicalDevice, vk::Device device, vk::CommandPool commandPool, vk::Queue queue) {
			uint32_t instanceCount = bufferManager->instanceData.size();
			std::vector<vk::AccelerationStructureInstanceKHR> accels;
			accels.reserve(instanceCount);

			for (int i = 0; i < bufferManager->instanceData.size(); i++) {
				auto& inst = bufferManager->instanceData[i];
				vk::AccelerationStructureInstanceKHR accel{};
				accel.setTransform(inst.transform);
				accel.setInstanceCustomIndex(i);
				accel.setMask(0xff);
				accel.setInstanceShaderBindingTableRecordOffset(0);
				accel.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable);
				accel.setAccelerationStructureReference(BLASes[inst.geometryIndex].buffer.address);

				accels.push_back(accel);
			}

			Buffer instanceBuffer;
			instanceBuffer.init(physicalDevice, device,
				sizeof(vk::AccelerationStructureInstanceKHR) * accels.size(),
				vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				accels.data()
			);

			vk::AccelerationStructureGeometryInstancesDataKHR instancesData{};
			instancesData.setArrayOfPointers(false);
			instancesData.setData(instanceBuffer.address);

			vk::AccelerationStructureGeometryKHR ias{};
			ias.setGeometryType(vk::GeometryTypeKHR::eInstances);
			ias.setGeometry({ instancesData });
			ias.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

			TLAS.init(
				physicalDevice, device, commandPool, queue,
				vk::AccelerationStructureTypeKHR::eTopLevel,
				ias, instanceCount
			);

		}

		void ReleaseTLAS(vk::Device device) {
			TLAS.Release(device);
		}

		void ReleaseBLAS(vk::Device device) {
			for (auto& blas : BLASes) {
				blas.Release(device);
			}
		}

		std::vector<AccelStruct> BLASes;
		AccelStruct TLAS;

	};
}

