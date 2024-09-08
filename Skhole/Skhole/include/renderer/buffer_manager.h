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
			uint32_t vertexOffset;
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
			vk::TransformMatrixKHR transform;
			vk::TransformMatrixKHR normalTransform;
		};

		void SetScene(ShrPtr<Scene> in_scene) {
			scene = in_scene;
		}

		void InitGeometryBuffer(vk::PhysicalDevice physicalDevice, vk::Device device, vk::CommandPool commandPool, vk::Queue queue) {

			auto geometries = scene->m_geometies;

			uint32_t vertexCount = 0;
			uint32_t indexCount = 0;
			uint32_t matIndexCount = 0;

			for (auto& geometry : geometries) {
				auto& vertices = geometry->m_vertices;
				auto& indices = geometry->m_indices;
				auto& matIndices = geometry->m_materialIndices;

				vertexCount += (uint32_t)vertices.size();
				indexCount += (uint32_t)indices.size();
				matIndexCount += (uint32_t)matIndices.size();
			}

			vk::BufferUsageFlags bufferUsage{
				vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
				vk::BufferUsageFlagBits::eStorageBuffer |
				vk::BufferUsageFlagBits::eShaderDeviceAddress
			};

			vk::MemoryPropertyFlags memoryProperty{
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			};

			vertexBuffer.Init(
				physicalDevice, device,
				vertexCount * sizeof(VertexData),
				bufferUsage, memoryProperty
			);

			indexBuffer.Init(
				physicalDevice, device,
				indexCount * sizeof(uint32_t),
				bufferUsage, memoryProperty
			);

			matIndexBuffer.Init(
				physicalDevice, device,
				matIndexCount * sizeof(uint32_t),
				bufferUsage, memoryProperty
			);

			uint32_t vertOffsetByte = 0;
			uint32_t indexOffsetByte = 0;
			uint32_t matIndexOffsetByte = 0;

			uint32_t indexOffset = 0;
			uint32_t vertexOffset = 0;

			for (auto& geometry : geometries) {
				auto& vertices = geometry->m_vertices;
				auto& indices = geometry->m_indices;
				auto& matIndices = geometry->m_materialIndices;

				void* vertexMap = vertexBuffer.Map(device, vertOffsetByte, vertices.size() * sizeof(VertexData));
				void* indexMap = indexBuffer.Map(device, indexOffsetByte, indices.size() * sizeof(uint32_t));
				void* matIndexMap = matIndexBuffer.Map(device, matIndexOffsetByte, geometry->m_materialIndices.size() * sizeof(uint32_t));

				memcpy(vertexMap, vertices.data(), vertices.size() * sizeof(VertexData));
				memcpy(indexMap, indices.data(), indices.size() * sizeof(uint32_t));
				memcpy(matIndexMap, matIndices.data(), matIndices.size() * sizeof(uint32_t));

				vertexBuffer.Unmap(device);
				indexBuffer.Unmap(device);
				matIndexBuffer.Unmap(device);

				GeometryData geomData;
				geomData.vertexOffset = vertexOffset;
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
				matIndexOffsetByte += matIndices.size() * sizeof(uint32_t);

				indexOffset += indices.size();
				vertexOffset += vertices.size();
			}

			vertexBuffer.UploadToDevice(device, commandPool, queue);
			indexBuffer.UploadToDevice(device, commandPool, queue);
			matIndexBuffer.UploadToDevice(device, commandPool, queue);

			vk::BufferUsageFlags geometryBufferUsage{
				vk::BufferUsageFlagBits::eStorageBuffer |
				vk::BufferUsageFlagBits::eShaderDeviceAddress
			};

			vk::MemoryPropertyFlags geometryBufferProperty{
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			};

			uint32_t geometryBufferSize = geometryData.size() * sizeof(GeometryData);
			geometryBuffer.Init(
				physicalDevice, device,
				geometryBufferSize,
				geometryBufferUsage, geometryBufferProperty
			);

			void* geometryMap = geometryBuffer.Map(device, 0, geometryBufferSize);
			memcpy(geometryMap, geometryData.data(), geometryBufferSize);
			geometryBuffer.Unmap(device);

			geometryBuffer.UploadToDevice(device, commandPool, queue);
		}

		void InitInstanceBuffer(vk::PhysicalDevice physicalDevice, vk::Device device, vk::CommandPool commandPool, vk::Queue queue) {
			auto& objects = scene->m_objects;

			for (auto& object : objects) {
				if (ObjectType::INSTANCE == object->GetObjectType()) {
					auto instance = std::static_pointer_cast<Instance>(object);

					InstanceData instData;
					instData.geometryIndex = instance->geometryIndex.value();

					instData.transform = std::array{
						std::array{1.0f, 0.0f, 0.0f, 0.0f},
						std::array{0.0f, 1.0f, 0.0f, 0.0f},
						std::array{0.0f, 0.0f, 1.0f, 0.0f}
					};

					instData.normalTransform = std::array{
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

			uint32_t instanceBufferSize = instanceData.size() * sizeof(InstanceData);

			instanceBuffer.Init(
				physicalDevice, device,
				instanceBufferSize,
				bufferUsage, memoryProperty
			);

			void* instanceMap = instanceBuffer.Map(device, 0, instanceBufferSize);
			memcpy(instanceMap, instanceData.data(), instanceBufferSize);
			instanceBuffer.Unmap(device);

			instanceBuffer.UploadToDevice(device, commandPool, queue);
		}

		void FrameUpdateInstance(float frame, vk::Device device, vk::CommandPool commandPool, vk::Queue queue) {
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
					mat3 normalTransform = NormalTransformMatrix3x3(transform);

					instData.transform = std::array{
						std::array{transform[0][0], transform[0][1], transform[0][2], transform[0][3]},
						std::array{transform[1][0], transform[1][1], transform[1][2], transform[1][3]},
						std::array{transform[2][0], transform[2][1], transform[2][2], transform[2][3]}
					};


					instData.normalTransform = std::array{
						std::array{normalTransform[0][0], normalTransform[0][1], normalTransform[0][2], 0.0f},
						std::array{normalTransform[1][0], normalTransform[1][1], normalTransform[1][2], 0.0f},
						std::array{normalTransform[2][0], normalTransform[2][1], normalTransform[2][2], 0.0f}
					};

					instanceData.push_back(instData);
				}
			}

			void* instanceMap = instanceBuffer.Map(device, 0, instanceBuffer.GetBufferSize());
			memcpy(instanceMap, instanceData.data(), instanceBuffer.GetBufferSize());
			instanceBuffer.Unmap(device);

			instanceBuffer.UploadToDevice(device, commandPool, queue);
		}

		void Release(vk::Device device) {
			vertexBuffer.Release(device);
			indexBuffer.Release(device);

			geometryBuffer.Release(device);
			instanceBuffer.Release(device);

			geometryOffset.clear();
			geometryData.clear();
			instanceData.clear();
		}

		DeviceBuffer vertexBuffer;
		DeviceBuffer indexBuffer;
		DeviceBuffer matIndexBuffer;

		std::vector<GeometryData> geometryData;
		std::vector<GeometryBufferData> geometryOffset;

		std::vector<InstanceData> instanceData;

		DeviceBuffer geometryBuffer;
		DeviceBuffer instanceBuffer;

		ShrPtr<Scene> scene = nullptr;
	};

	class ASManager {
	public:
		ASManager() {};
		~ASManager() {};

		void BuildBLAS(SceneBufferaManager& bufferManager, vk::PhysicalDevice physicalDevice, vk::Device device, vk::CommandPool commandPool, vk::Queue queue) {
			auto& geomOffset = bufferManager.geometryOffset;
			BLASes.clear();
			BLASes.resize(geomOffset.size());

			for (int i = 0; i < geomOffset.size(); i++) {
				auto& geom = geomOffset[i];
				vk::AccelerationStructureGeometryTrianglesDataKHR triangles{};
				triangles.setVertexFormat(vk::Format::eR32G32B32Sfloat);
				triangles.setVertexData(bufferManager.vertexBuffer.GetDeviceAddress() + geom.vertexOffsetByte);
				triangles.setVertexStride(sizeof(VertexData));
				triangles.setMaxVertex(geom.numVert);
				triangles.setIndexType(vk::IndexType::eUint32);
				triangles.setIndexData(bufferManager.indexBuffer.GetDeviceAddress() + geom.indexOffsetByte);

				vk::AccelerationStructureGeometryKHR geometry{};
				geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
				geometry.setGeometry({ triangles });
				geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

				uint32_t primitiveCount = geom.numIndex / 3;

				BLASes[i].init(physicalDevice, device, commandPool, queue,
					vk::AccelerationStructureTypeKHR::eBottomLevel,
					geometry, primitiveCount);
			}
		}

		void UpdateBLAS() {
			SKHOLE_UNIMPL("Update BLAS");
		}

		void BuildTLAS(SceneBufferaManager& bufferManager, vk::PhysicalDevice physicalDevice, vk::Device device, vk::CommandPool commandPool, vk::Queue queue) {
			uint32_t instanceCount = bufferManager.instanceData.size();
			std::vector<vk::AccelerationStructureInstanceKHR> accels;
			accels.reserve(instanceCount);

			for (int i = 0; i < bufferManager.instanceData.size(); i++) {
				auto& inst = bufferManager.instanceData[i];
				vk::AccelerationStructureInstanceKHR accel{};
				//vk::TransformMatrixKHR transform = std::array{
				//	std::array{ 1.0f, 0.0f, 0.0f, 0.0f }, 
				//	std::array{ 0.0f, 1.0f, 0.0f, 0.0f }, 
				//	std::array{ 0.0f, 0.0f, 1.0f, 0.0f }, 
				//};
				//accel.setTransform(transform);
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

