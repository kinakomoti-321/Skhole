#include <renderer/simple_raytracer.h>
#include <common/log.h>
#include <vulkan_helpler/vkhelper.h>

namespace Skhole {

	SimpleRaytracer::SimpleRaytracer()
	{

	}

	SimpleRaytracer::~SimpleRaytracer()
	{
	}

	void SimpleRaytracer::Init(RendererDesc& desc)
	{
		SKHOLE_LOG_SECTION("Initialze Renderer");
		m_instance = VKHelper::CreateInstance(VK_API_VERSION_1_2, m_layer);
		m_debugMessenger = VKHelper::CreateDebugMessenger(*m_instance);

		m_surface = VKHelper::CreateSurface(*m_instance, desc.window);

		m_physicalDevice = VKHelper::PickPhysicalDevice(*m_instance, *m_surface, m_extension);

		m_graphicsQueueIndex = VKHelper::FindGeneralQueueFamily(m_physicalDevice, *m_surface);
		m_device = VKHelper::CreateLogicalDevice(m_physicalDevice, m_graphicsQueueIndex, m_extension);
		m_graphicsQueue = m_device->getQueue(m_graphicsQueueIndex, 0);

		m_commandPool = VKHelper::CreateCommandPool(*m_device, m_graphicsQueueIndex);
		m_commandBuffer = VKHelper::CreateCommandBuffer(*m_device, *m_commandPool);

		m_surfaceFormat = VKHelper::ChooseSurfaceFormat(m_physicalDevice, *m_surface, vk::Format::eR8G8B8A8Unorm);
		m_swapchain = VKHelper::CreateSwapchain(
			m_physicalDevice, *m_device, *m_surface,
			vk::ImageUsageFlagBits::eColorAttachment,
			m_surfaceFormat, desc.Width, desc.Height, m_graphicsQueueIndex);

		m_swapchainImages = m_device->getSwapchainImagesKHR(*m_swapchain);

		SKHOLE_LOG("Create Swapchain Image View");
		for (auto& image : m_swapchainImages) {
			vk::ImageViewCreateInfo imageViewCreateInfo;
			imageViewCreateInfo.setImage(image);
			imageViewCreateInfo.setViewType(vk::ImageViewType::e2D);
			imageViewCreateInfo.setFormat(m_surfaceFormat.format);
			imageViewCreateInfo.setComponents({ vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA });
			imageViewCreateInfo.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

			m_swapchainImageViews.push_back(
				m_device->createImageViewUnique(imageViewCreateInfo)
			);
		}

		VKHelper::OneTimeSubmit(*m_device, *m_commandPool, m_graphicsQueue,
			[&](vk::CommandBuffer commandBuffer) {
				for (auto image : m_swapchainImages) {
					VKHelper::SetImageLayout(commandBuffer, image, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
				}
			}
		);


		SKHOLE_LOG("Buttom AS Swapchain Image View");
		std::vector<Vertex> vertices = {
			{{1.0f, 1.0f, 0.0f}},
			{{-1.0f, 1.0f, 0.0f}},
			{{0.0f, -1.0f, 0.0f}},
		};
		std::vector<uint32_t> indices = { 0, 1, 2 };

		vk::BufferUsageFlags bufferUsage{
			vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
			vk::BufferUsageFlagBits::eShaderDeviceAddress
		};

		vk::MemoryPropertyFlags memoryProperty{
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		};

		Buffer vertexBuffer;
		vertexBuffer.init(m_physicalDevice, *m_device, vertices.size() * sizeof(Vertex), bufferUsage, memoryProperty, vertices.data());

		Buffer indexBuffer;
		indexBuffer.init(m_physicalDevice, *m_device, indices.size() * sizeof(uint32_t), bufferUsage, memoryProperty, indices.data());

		vk::AccelerationStructureGeometryTrianglesDataKHR triangles{};
		triangles.setVertexFormat(vk::Format::eR32G32B32Sfloat);
		triangles.setVertexData(vertexBuffer.address);
		triangles.setVertexStride(sizeof(Vertex));
		triangles.setMaxVertex(vertices.size());
		triangles.setIndexType(vk::IndexType::eUint32);
		triangles.setIndexData(indexBuffer.address);

		vk::AccelerationStructureGeometryKHR geometry{};
		geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
		geometry.setGeometry({ triangles });
		geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

		uint32_t primitiveCount = static_cast<uint32_t>(indices.size() / 3);
		m_bottomAccel.init(m_physicalDevice, *m_device, *m_commandPool, m_graphicsQueue, vk::AccelerationStructureTypeKHR::eBottomLevel, geometry, primitiveCount);

		SKHOLE_LOG("Bottom AS was Created");


		vk::TransformMatrixKHR transform = std::array{
			std::array{1.0f, 0.0f, 0.0f, 0.0f},
			std::array{0.0f, 1.0f, 0.0f, 0.0f},
			std::array{0.0f, 0.0f, 1.0f, 0.0f}
		};

		vk::AccelerationStructureInstanceKHR accelInstance{};
		accelInstance.setTransform(transform);
		accelInstance.setInstanceCustomIndex(0);
		accelInstance.setMask(0xFF);
		accelInstance.setInstanceShaderBindingTableRecordOffset(0);
		accelInstance.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable);
		accelInstance.setAccelerationStructureReference(m_bottomAccel.buffer.address);

		Buffer instanceBuffer;
		instanceBuffer.init(m_physicalDevice, *m_device,
			sizeof(vk::AccelerationStructureInstanceKHR),
			vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			&accelInstance
		);

		vk::AccelerationStructureGeometryInstancesDataKHR instancesData{};
		instancesData.setArrayOfPointers(false);
		instancesData.setData(instanceBuffer.address);

		vk::AccelerationStructureGeometryKHR ias{};
		ias.setGeometryType(vk::GeometryTypeKHR::eInstances);
		ias.setGeometry({ instancesData });
		ias.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

		constexpr uint32_t gasCount = 1;
		m_topAccel.init(
			m_physicalDevice, *m_device, *m_commandPool, m_graphicsQueue,
			vk::AccelerationStructureTypeKHR::eTopLevel,
			ias, gasCount);
		SKHOLE_LOG("Instance AS was Created");

		PrepareShader();
		CreateDescriptorPool();
		CreateDescSetLayout();
		CreateDescSet();

		CreatePipeline();

		SKHOLE_LOG_SECTION("Initialze Renderer Completed");
	}

#define SHADER_FILE_PATH "shader/"
	void SimpleRaytracer::AddShader(
		uint32_t shaderIndex,
		const std::string& shaderName,
		vk::ShaderStageFlagBits stage
	)
	{
		shaderModules[shaderIndex] = VKHelper::CreateShaderModule(*m_device, SHADER_FILE_PATH + shaderName);
		shaderStages[shaderIndex].setStage(stage);
		shaderStages[shaderIndex].setModule(*shaderModules[shaderIndex]);
		shaderStages[shaderIndex].setPName("main");
	}

	void SimpleRaytracer::PrepareShader() {
		SKHOLE_LOG("Prepare Shader");

		uint32_t raygenShader = 0;
		uint32_t missShader = 1;
		uint32_t closestHitShader = 2;

		shaderModules.resize(3);
		shaderStages.resize(3);

		//std::filesystem::path relativePath("shader/simple_raytracer/raygen.rgen.spv");
		//std::filesystem::path absolutePath = std::filesystem::absolute(relativePath);
		//std::cout << "Absolute path: " << absolutePath << std::endl;

		AddShader(raygenShader, "simple_raytracer/raygen.rgen.spv", vk::ShaderStageFlagBits::eRaygenKHR);
		AddShader(missShader, "simple_raytracer/miss.rmiss.spv", vk::ShaderStageFlagBits::eMissKHR);
		AddShader(closestHitShader, "simple_raytracer/closesthit.rchit.spv", vk::ShaderStageFlagBits::eClosestHitKHR);
	}

	void SimpleRaytracer::CreateDescriptorPool()
	{
		std::vector<vk::DescriptorPoolSize> poolSize = {
			{vk::DescriptorType::eAccelerationStructureKHR, 1},
			{vk::DescriptorType::eStorageImage, 1},
		};

		vk::DescriptorPoolCreateInfo poolCreateInfo;
		poolCreateInfo.setPoolSizes(poolSize);
		poolCreateInfo.setMaxSets(1);
		poolCreateInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
		descPool = m_device->createDescriptorPoolUnique(poolCreateInfo);
	}

	void SimpleRaytracer::CreateDescSetLayout() {
		std::vector<vk::DescriptorSetLayoutBinding> bindings(2);
		bindings[0].setBinding(0);
		bindings[0].setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR);
		bindings[0].setDescriptorCount(1);
		bindings[0].setStageFlags(vk::ShaderStageFlagBits::eRaygenKHR);

		bindings[1].setBinding(1);
		bindings[1].setDescriptorType(vk::DescriptorType::eStorageImage);
		bindings[1].setDescriptorCount(1);
		bindings[1].setStageFlags(vk::ShaderStageFlagBits::eRaygenKHR);

		vk::DescriptorSetLayoutCreateInfo layoutCreateInfo;
		layoutCreateInfo.setBindings(bindings);
		descSetLayout = m_device->createDescriptorSetLayoutUnique(layoutCreateInfo);
	}

	void SimpleRaytracer::CreateDescSet() {
		vk::DescriptorSetAllocateInfo allocateInfo;
		allocateInfo.setDescriptorPool(*descPool);
		allocateInfo.setSetLayouts(*descSetLayout);
		descSet = std::move(m_device->allocateDescriptorSetsUnique(allocateInfo).front());
	}

#define MAX_DEPTH 1
	void SimpleRaytracer::CreatePipeline() {

		vk::PipelineLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.setSetLayouts(*descSetLayout);
		m_pipelineLayout = m_device->createPipelineLayoutUnique(layoutCreateInfo);


		vk::RayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
		pipelineCreateInfo.setLayout(*m_pipelineLayout);
		pipelineCreateInfo.setStages(shaderStages);
		pipelineCreateInfo.setGroups(shaderGroups);
		pipelineCreateInfo.setMaxPipelineRayRecursionDepth(MAX_DEPTH);

		auto result = m_device->createRayTracingPipelineKHRUnique(nullptr, nullptr, pipelineCreateInfo);
		if (result.result != vk::Result::eSuccess) {
			SKHOLE_ABORT("Failed to create raytracing pipeline");
		}

		m_pipeline = std::move(result.value);
	}

	void SimpleRaytracer::CreateShaderBindingTable() {
		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties = VKHelper::GetRayTracingProp(m_physicalDevice);

		uint32_t handleSize = rtProperties.shaderGroupHandleSize;
		uint32_t handleAlignment = rtProperties.shaderGroupHandleAlignment;
		uint32_t baseAlignment = rtProperties.shaderGroupBaseAlignment;
		uint32_t handleSizeAligned = VKHelper::AlignUp(handleSize, handleAlignment);

		uint32_t raygenShaderCount = 1;
		uint32_t missShaderCount = 1;
		uint32_t hitShaderCount = 1;

		raygenRegion.setStride(VKHelper::AlignUp(handleSizeAligned, baseAlignment));
		raygenRegion.setSize(raygenRegion.stride);

		missRegion.setStride(handleSizeAligned);
		missRegion.setSize(VKHelper::AlignUp(missShaderCount * handleSizeAligned,
			baseAlignment));

		hitRegion.setStride(handleSizeAligned);
		hitRegion.setSize(VKHelper::AlignUp(hitShaderCount * handleSizeAligned,
			baseAlignment));

		vk::DeviceSize stbSize = raygenRegion.size + missRegion.size + hitRegion.size;
		sbt.init(m_physicalDevice, *m_device, stbSize,
			vk::BufferUsageFlagBits::eShaderBindingTableKHR |
			vk::BufferUsageFlagBits::eTransferSrc |
			vk::BufferUsageFlagBits::eShaderDeviceAddress,
			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent
		);

		uint32_t handleCount = raygenShaderCount + missShaderCount + hitShaderCount;
		uint32_t handleStorageSize = handleCount * handleSize;

		std::vector<uint8_t> handleStorage(handleStorageSize);

		auto result = m_device->getRayTracingShaderGroupHandlesKHR(
			*m_pipeline, 0, handleCount, handleStorageSize, handleStorage.data()
		);

		if (result != vk::Result::eSuccess) {
			SKHOLE_ABORT("Failed to get shader group handles");
		}

		uint8_t* sbtHead = static_cast<uint8_t*>(m_device->mapMemory(*sbt.memory, 0, stbSize));
		uint8_t* dstPtr = sbtHead;

		auto copyHandle = [&](uint32_t index) {
			std::memcpy(dstPtr, handleStorage.data() + handleSize * index, handleSize);
			};

		uint32_t handleIndex = 0;
		copyHandle(handleIndex++);

		dstPtr = sbtHead + raygenRegion.size;
		for (uint32_t c = 0; c < missShaderCount; c++) {
			copyHandle(handleIndex++);
			dstPtr += missRegion.stride;
		}

		dstPtr = sbtHead + raygenRegion.size + missRegion.size;
		for (uint32_t c = 0; c < hitShaderCount; c++) {
			copyHandle(handleIndex++);
			dstPtr += hitRegion.stride;
		}

		raygenRegion.setDeviceAddress(sbt.address);
		missRegion.setDeviceAddress(sbt.address + raygenRegion.size);
		hitRegion.setDeviceAddress(sbt.address + raygenRegion.size + missRegion.size);
	}

	void SimpleRaytracer::Destroy()
	{
		m_device->waitIdle();

		m_swapchainImageViews.clear();
		m_swapchain.reset();
		m_commandBuffer.reset();
		m_commandPool.reset();
		m_device.reset();
		m_physicalDevice = vk::PhysicalDevice();
		m_surface.reset();
		m_debugMessenger.reset();
		m_instance.reset();
	}

	void SimpleRaytracer::Resize(unsigned int width, unsigned int height)
	{
		SKHOLE_UNIMPL("Resize");
	}

	void SimpleRaytracer::Render()
	{
		static int frame = 0;
		frame++;

		vk::UniqueSemaphore imageAvailableSemaphre = m_device->createSemaphoreUnique({});

		auto result = m_device->acquireNextImageKHR(
			*m_swapchain, std::numeric_limits<uint64_t>::max(), *imageAvailableSemaphre
		);
		if (result.result != vk::Result::eSuccess) {
			SKHOLE_ABORT("Failed to acquire next image");
		}

		uint32_t imageIndex = result.value;

		UpdateDescriptorSet(*m_swapchainImageViews[imageIndex]);

		RecordCommandBuffer(m_swapchainImages[imageIndex]);

		vk::PipelineStageFlags waitStage{vk::PipelineStageFlagBits::eTopOfPipe};
		vk::SubmitInfo submitInfo;
		submitInfo.setWaitDstStageMask(waitStage);
		submitInfo.setCommandBuffers(*m_commandBuffer);
		submitInfo.setWaitSemaphores(*imageAvailableSemaphre);
		m_graphicsQueue.submit(submitInfo, nullptr);

		m_graphicsQueue.waitIdle();

		vk::PresentInfoKHR presentInfo{};
		presentInfo.setSwapchains(*m_swapchain);
		presentInfo.setImageIndices(imageIndex);

		if (m_graphicsQueue.presentKHR(presentInfo) != vk::Result::eSuccess) {
			SKHOLE_ABORT("Failed to present image");
		}
	}

	void SimpleRaytracer::RecordCommandBuffer(vk::Image image) {
		m_commandBuffer->begin(vk::CommandBufferBeginInfo{});
		VKHelper::SetImageLayout(*m_commandBuffer, image, vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eGeneral);

		m_commandBuffer->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, *m_pipeline);

		m_commandBuffer->bindDescriptorSets(
			vk::PipelineBindPoint::eRayTracingKHR,
			*m_pipelineLayout,
			0,
			*descSet,
			nullptr
		);

		m_commandBuffer->traceRaysKHR(
			raygenRegion,
			missRegion,
			hitRegion,
			{},
			m_desc.Width, m_desc.Height, 1
		);

		VKHelper::SetImageLayout(*m_commandBuffer, image, vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR);

		m_commandBuffer->end();

	}

	void SimpleRaytracer::UpdateDescriptorSet(vk::ImageView imageView) {
		std::vector<vk::WriteDescriptorSet> writes(2);

		vk::WriteDescriptorSetAccelerationStructureKHR accelInfo{};
		accelInfo.setAccelerationStructures(*m_topAccel.accel);
		writes[0].setDstSet(*descSet);
		writes[0].setDstBinding(0);
		writes[0].setDescriptorCount(1);
		writes[0].setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR);
		writes[0].setPNext(&accelInfo);

		vk::DescriptorImageInfo imageInfo{};
		imageInfo.setImageView(imageView);
		imageInfo.setImageLayout(vk::ImageLayout::eGeneral);
		writes[1].setDstSet(*descSet);
		writes[1].setDstBinding(1);
		writes[1].setDescriptorCount(1);
		writes[1].setDescriptorType(vk::DescriptorType::eStorageImage);
		writes[1].setPImageInfo(&imageInfo);

		m_device->updateDescriptorSets(writes, nullptr);
	}


	void SimpleRaytracer::OffscreenRender()
	{
		SKHOLE_UNIMPL("OffscreenRender");
	}

	RendererData SimpleRaytracer::GetRendererData()
	{
		return RendererData();
	}

	void SimpleRaytracer::InitVulkan()
	{
		SKHOLE_UNIMPL("InitVulkan");
	}
}
