#include <renderer/simple_raytracer.h>
#include <common/log.h>
#include <vulkan_helpler/vkutils.hpp>

namespace Skhole {

	SimpleRaytracer::SimpleRaytracer()
	{

	}

	SimpleRaytracer::~SimpleRaytracer()
	{

	}

	void SimpleRaytracer::Wait() {
		m_device->waitIdle();
	}

	void SimpleRaytracer::InitImGui()
	{
		m_context = ImGui::CreateContext();
		ImGui::SetCurrentContext(m_context);
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForVulkan(m_desc.window, true);

		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = *m_instance;
		init_info.PhysicalDevice = m_physicalDevice;
		init_info.Device = *m_device;
		init_info.QueueFamily = m_queueIndex;
		init_info.Queue = m_queue;
		init_info.PipelineCache = VK_NULL_HANDLE;

		// Not share
		std::vector<vk::DescriptorPoolSize> poolSize = {
			{vk::DescriptorType::eSampler,1000},
			{vk::DescriptorType::eCombinedImageSampler,1000},
			{vk::DescriptorType::eSampledImage,1000},
			{vk::DescriptorType::eStorageImage,1000},
			{vk::DescriptorType::eUniformTexelBuffer,1000},
			{vk::DescriptorType::eStorageTexelBuffer,1000},
			{vk::DescriptorType::eUniformBuffer,1000},
			{vk::DescriptorType::eStorageBuffer,1000},
			{vk::DescriptorType::eUniformBufferDynamic,1000},
			{vk::DescriptorType::eStorageBufferDynamic,1000},
			{vk::DescriptorType::eInputAttachment,1000}
		};

		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
		poolInfo.setMaxSets(1000);
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
		poolInfo.pPoolSizes = poolSize.data();

		m_imGuiDescriptorPool = m_device->createDescriptorPoolUnique(poolInfo);

		init_info.DescriptorPool = *m_imGuiDescriptorPool;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = 2;
		init_info.ImageCount = m_swapchainImages.size();
		init_info.CheckVkResultFn = nullptr;

		//Create RenderPass;
		vk::AttachmentDescription colorAttachment = {};
		colorAttachment.setFormat(vk::Format::eB8G8R8A8Unorm);
		colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
		colorAttachment.setLoadOp(vk::AttachmentLoadOp::eLoad);
		colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
		colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
		colorAttachment.setInitialLayout(vk::ImageLayout::eGeneral);
		colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		vk::AttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.setAttachment(0);
		colorAttachmentRef.setLayout(vk::ImageLayout::eGeneral);

		vk::SubpassDescription subpass = {};
		subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics); //Utagai
		subpass.setColorAttachmentCount(1);
		subpass.setPColorAttachments(&colorAttachmentRef);

		vk::RenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.setAttachmentCount(1);
		renderPassInfo.setPAttachments(&colorAttachment);
		renderPassInfo.setSubpassCount(1);
		renderPassInfo.setPSubpasses(&subpass);

		m_imGuiRenderPass = m_device->createRenderPassUnique(renderPassInfo);

		init_info.RenderPass = *m_imGuiRenderPass;
		ImGui_ImplVulkan_Init(&init_info);

		ImGui_ImplVulkan_CreateFontsTexture();

	}

	void SimpleRaytracer::Init(RendererDesc& desc)
	{
		SKHOLE_LOG_SECTION("Initialze Renderer");
		m_desc = desc;

		m_instance = vkutils::createInstance(VK_API_VERSION_1_2, m_layer);
		m_debugMessenger = vkutils::createDebugMessenger(*m_instance);

		m_surface = vkutils::createSurface(*m_instance, desc.window);

		m_physicalDevice = vkutils::pickPhysicalDevice(*m_instance, *m_surface, m_extension);

		m_queueIndex = vkutils::findGeneralQueueFamily(m_physicalDevice, *m_surface);
		m_device = vkutils::createLogicalDevice(m_physicalDevice, m_queueIndex, m_extension);
		m_queue = m_device->getQueue(m_queueIndex, 0);

		m_commandPool = vkutils::createCommandPool(*m_device, m_queueIndex);
		m_commandBuffer = vkutils::createCommandBuffer(*m_device, *m_commandPool);

		m_surfaceFormat = vkutils::chooseSurfaceFormat(m_physicalDevice, *m_surface);

		m_swapchain = vkutils::createSwapchain(  //
			m_physicalDevice, *m_device, *m_surface, m_queueIndex,
			vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment, m_surfaceFormat,  //
			m_desc.Width, m_desc.Height);

		m_swapchainImages = m_device->getSwapchainImagesKHR(*m_swapchain);

		SKHOLE_LOG("Create Swapchain Image View");
		for (auto& image : m_swapchainImages) {

			vk::ImageViewCreateInfo createInfo{};
			createInfo.setImage(image);
			createInfo.setViewType(vk::ImageViewType::e2D);
			createInfo.setFormat(m_surfaceFormat.format);
			createInfo.setComponents(
				{ vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
					vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA });
			createInfo.setSubresourceRange(
				{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

			m_swapchainImageViews.push_back(
				m_device->createImageViewUnique(createInfo));
		}




		vkutils::oneTimeSubmit(*m_device, *m_commandPool, m_queue,
			[&](vk::CommandBuffer commandBuffer) {
				for (auto& image : m_swapchainImages) {
					vkutils::setImageLayout(commandBuffer, image,
						vk::ImageLayout::eUndefined,
						vk::ImageLayout::ePresentSrcKHR);
				}
			});

		CreateBottomLevelAS();
		CreateTopLevelAS();

		PrepareShader();

		CreateDescriptorPool();
		CreateDescSetLayout();
		CreateDescSet();

		CreatePipeline();
		CreateShaderBindingTable();

		InitImGui();

		m_frameBuffer.resize(m_swapchainImageViews.size());
		for (int i = 0; i < m_swapchainImageViews.size(); i++) {
			vk::ImageView attachments[] = {
				*m_swapchainImageViews[i]
			};

			vk::FramebufferCreateInfo framebufferInfo{};
			framebufferInfo.setRenderPass(*m_imGuiRenderPass);
			framebufferInfo.setAttachmentCount(1);
			framebufferInfo.setPAttachments(attachments);
			framebufferInfo.setWidth(m_desc.Width);
			framebufferInfo.setHeight(m_desc.Height);
			framebufferInfo.setLayers(1);

			m_frameBuffer[i] = m_device->createFramebufferUnique(framebufferInfo);
		}

		SKHOLE_LOG_SECTION("Initialze Renderer Completed");
	}

	void SimpleRaytracer::CreateBottomLevelAS() {
		SKHOLE_LOG("Create Buttom AS");
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
		m_bottomAccel.init(m_physicalDevice, *m_device, *m_commandPool, m_queue, vk::AccelerationStructureTypeKHR::eBottomLevel, geometry, primitiveCount);
	}

	void SimpleRaytracer::CreateTopLevelAS() {
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
			m_physicalDevice, *m_device, *m_commandPool, m_queue,
			vk::AccelerationStructureTypeKHR::eTopLevel,
			ias, gasCount);
		SKHOLE_LOG("Instance AS was Created");
	}

#define SHADER_FILE_PATH "shader/"
	void SimpleRaytracer::AddShader(
		uint32_t shaderIndex,
		const std::string& shaderName,
		vk::ShaderStageFlagBits stage
	)
	{
		shaderModules[shaderIndex] = vkutils::createShaderModule(*m_device, SHADER_FILE_PATH + shaderName);
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

		AddShader(raygenShader, "simple_raytracer/raygen.rgen.spv", vk::ShaderStageFlagBits::eRaygenKHR);
		AddShader(missShader, "simple_raytracer/miss.rmiss.spv", vk::ShaderStageFlagBits::eMissKHR);
		AddShader(closestHitShader, "simple_raytracer/closesthit.rchit.spv", vk::ShaderStageFlagBits::eClosestHitKHR);

		uint32_t raygenGroup = 0;
		uint32_t missGroup = 1;
		uint32_t hitGroup = 2;

		shaderGroups.resize(3);

		// Raygen group
		shaderGroups[raygenGroup].setType(
			vk::RayTracingShaderGroupTypeKHR::eGeneral);
		shaderGroups[raygenGroup].setGeneralShader(raygenShader);
		shaderGroups[raygenGroup].setClosestHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[raygenGroup].setAnyHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[raygenGroup].setIntersectionShader(VK_SHADER_UNUSED_KHR);

		// Miss group
		shaderGroups[missGroup].setType(
			vk::RayTracingShaderGroupTypeKHR::eGeneral);
		shaderGroups[missGroup].setGeneralShader(missShader);
		shaderGroups[missGroup].setClosestHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[missGroup].setAnyHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[missGroup].setIntersectionShader(VK_SHADER_UNUSED_KHR);

		// Hit group
		shaderGroups[hitGroup].setType(
			vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup);
		shaderGroups[hitGroup].setGeneralShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[hitGroup].setClosestHitShader(closestHitShader);
		shaderGroups[hitGroup].setAnyHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[hitGroup].setIntersectionShader(VK_SHADER_UNUSED_KHR);
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
		bindings[0].setDescriptorType(
			vk::DescriptorType::eAccelerationStructureKHR);
		bindings[0].setDescriptorCount(1);
		bindings[0].setStageFlags(vk::ShaderStageFlagBits::eRaygenKHR);

		bindings[1].setBinding(1);
		bindings[1].setDescriptorType(vk::DescriptorType::eStorageImage);
		bindings[1].setDescriptorCount(1);
		bindings[1].setStageFlags(vk::ShaderStageFlagBits::eRaygenKHR);

		vk::DescriptorSetLayoutCreateInfo createInfo{};
		createInfo.setBindings(bindings);
		descSetLayout = m_device->createDescriptorSetLayoutUnique(createInfo);
	}

	void SimpleRaytracer::CreateDescSet() {
		std::cout << "Create desc set\n";

		vk::DescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.setDescriptorPool(*descPool);
		allocateInfo.setSetLayouts(*descSetLayout);
		descSet = std::move(
			m_device->allocateDescriptorSetsUnique(allocateInfo).front());
	}

	void SimpleRaytracer::CreatePipeline() {
		std::cout << "Create pipeline\n";

		// Create pipeline layout
		vk::PipelineLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.setSetLayouts(*descSetLayout);
		m_pipelineLayout = m_device->createPipelineLayoutUnique(layoutCreateInfo);

		// Create pipeline
		vk::RayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
		pipelineCreateInfo.setLayout(*m_pipelineLayout);
		pipelineCreateInfo.setStages(shaderStages);
		pipelineCreateInfo.setGroups(shaderGroups);
		pipelineCreateInfo.setMaxPipelineRayRecursionDepth(1);
		auto result = m_device->createRayTracingPipelineKHRUnique(
			nullptr, nullptr, pipelineCreateInfo);
		if (result.result != vk::Result::eSuccess) {
			std::cerr << "Failed to create ray tracing pipeline.\n";
			std::abort();
		}
		m_pipeline = std::move(result.value);
	}

	void SimpleRaytracer::CreateShaderBindingTable() {
		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties =
			vkutils::getRayTracingProps(m_physicalDevice);
		uint32_t handleSize = rtProperties.shaderGroupHandleSize;
		uint32_t handleAlignment = rtProperties.shaderGroupHandleAlignment;
		uint32_t baseAlignment = rtProperties.shaderGroupBaseAlignment;
		uint32_t handleSizeAligned =
			vkutils::alignUp(handleSize, handleAlignment);

		// Set strides and sizes
		uint32_t raygenShaderCount = 1;  // raygen count must be 1
		uint32_t missShaderCount = 1;
		uint32_t hitShaderCount = 1;

		raygenRegion.setStride(
			vkutils::alignUp(handleSizeAligned, baseAlignment));
		raygenRegion.setSize(raygenRegion.stride);

		missRegion.setStride(handleSizeAligned);
		missRegion.setSize(vkutils::alignUp(missShaderCount * handleSizeAligned,
			baseAlignment));

		hitRegion.setStride(handleSizeAligned);
		hitRegion.setSize(vkutils::alignUp(hitShaderCount * handleSizeAligned,
			baseAlignment));

		// Create SBT
		vk::DeviceSize sbtSize =
			raygenRegion.size + missRegion.size + hitRegion.size;
		sbt.init(m_physicalDevice, *m_device, sbtSize,
			vk::BufferUsageFlagBits::eShaderBindingTableKHR |
			vk::BufferUsageFlagBits::eTransferSrc |
			vk::BufferUsageFlagBits::eShaderDeviceAddress,
			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent);

		// Get shader group handles
		uint32_t handleCount =
			raygenShaderCount + missShaderCount + hitShaderCount;
		uint32_t handleStorageSize = handleCount * handleSize;
		std::vector<uint8_t> handleStorage(handleStorageSize);

		auto result = m_device->getRayTracingShaderGroupHandlesKHR(
			*m_pipeline, 0, handleCount, handleStorageSize, handleStorage.data());
		if (result != vk::Result::eSuccess) {
			std::cerr << "Failed to get ray tracing shader group handles.\n";
			std::abort();
		}

		// Copy handles
		uint32_t handleIndex = 0;
		uint8_t* sbtHead =
			static_cast<uint8_t*>(m_device->mapMemory(*sbt.memory, 0, sbtSize));

		uint8_t* dstPtr = sbtHead;
		auto copyHandle = [&](uint32_t index) {
			std::memcpy(dstPtr, handleStorage.data() + handleSize * index,
				handleSize);
			};

		// Raygen
		copyHandle(handleIndex++);

		// Miss
		dstPtr = sbtHead + raygenRegion.size;
		for (uint32_t c = 0; c < missShaderCount; c++) {
			copyHandle(handleIndex++);
			dstPtr += missRegion.stride;
		}

		// Hit
		dstPtr = sbtHead + raygenRegion.size + missRegion.size;
		for (uint32_t c = 0; c < hitShaderCount; c++) {
			copyHandle(handleIndex++);
			dstPtr += hitRegion.stride;
		}

		raygenRegion.setDeviceAddress(sbt.address);
		missRegion.setDeviceAddress(sbt.address + raygenRegion.size);
		hitRegion.setDeviceAddress(sbt.address + raygenRegion.size +
			missRegion.size);
	}

	void SimpleRaytracer::Destroy()
	{
		m_device->waitIdle();


		ImGui_ImplVulkan_DestroyFontsTexture();
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

	}

	void SimpleRaytracer::Resize(unsigned int width, unsigned int height)
	{
		SKHOLE_UNIMPL("Resize");
	}


	void SimpleRaytracer::SetNewFrame()
	{
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui::NewFrame();
	}

	void SimpleRaytracer::Update()
	{

	}

	void SimpleRaytracer::Render()
	{

		static int frame = 0;

		// Create semaphore
		vk::UniqueSemaphore imageAvailableSemaphore =
			m_device->createSemaphoreUnique({});

		// Acquire next image
		auto result = m_device->acquireNextImageKHR(
			*m_swapchain, std::numeric_limits<uint64_t>::max(),
			*imageAvailableSemaphore);
		if (result.result != vk::Result::eSuccess) {
			std::cerr << "Failed to acquire next image.\n";
			std::abort();
		}

		// Update descriptor sets using current image
		uint32_t imageIndex = result.value;
		UpdateDescriptorSet(*m_swapchainImageViews[imageIndex]);

		// Record command buffer
		RecordCommandBuffer(m_swapchainImages[imageIndex], *m_frameBuffer[imageIndex]);

		// Submit command buffer
		vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eTopOfPipe };
		vk::SubmitInfo submitInfo{};
		submitInfo.setWaitDstStageMask(waitStage);
		submitInfo.setCommandBuffers(*m_commandBuffer);
		submitInfo.setWaitSemaphores(*imageAvailableSemaphore);
		m_queue.submit(submitInfo);

		// Wait
		m_queue.waitIdle();

		// Present
		vk::PresentInfoKHR presentInfo{};
		presentInfo.setSwapchains(*m_swapchain);
		presentInfo.setImageIndices(imageIndex);
		if (m_queue.presentKHR(presentInfo) != vk::Result::eSuccess) {
			std::cerr << "Failed to present.\n";
			std::abort();
		}

		frame++;
	}

	void SimpleRaytracer::RecordCommandBuffer(vk::Image image, vk::Framebuffer frameBuffer) {

		m_commandBuffer->begin(vk::CommandBufferBeginInfo{});
		vkutils::setImageLayout(*m_commandBuffer, image, vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eGeneral);

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


		//--------------------
		// ImGUI
		//--------------------
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.setRenderPass(*m_imGuiRenderPass);
		renderPassInfo.setFramebuffer(frameBuffer);
		vk::Rect2D rect({ 0,0 }, { (uint32_t)m_desc.Width,(uint32_t)m_desc.Height });

		renderPassInfo.setRenderArea(rect);

		m_commandBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *m_commandBuffer);

		m_commandBuffer->endRenderPass();

		//--------------------
		// ImGUI End
		//--------------------

		m_commandBuffer->end();

	}

	void SimpleRaytracer::UpdateDescriptorSet(vk::ImageView imageView) {
		std::vector<vk::WriteDescriptorSet> writes(2);

		// [0]: For AS
		vk::WriteDescriptorSetAccelerationStructureKHR accelInfo{};
		accelInfo.setAccelerationStructures(*m_topAccel.accel);
		writes[0].setDstSet(*descSet);
		writes[0].setDstBinding(0);
		writes[0].setDescriptorCount(1);
		writes[0].setDescriptorType(
			vk::DescriptorType::eAccelerationStructureKHR);
		writes[0].setPNext(&accelInfo);

		// [0]: For storage image
		vk::DescriptorImageInfo imageInfo{};
		imageInfo.setImageView(imageView);
		imageInfo.setImageLayout(vk::ImageLayout::eGeneral);
		writes[1].setDstSet(*descSet);
		writes[1].setDstBinding(1);
		writes[1].setDescriptorType(vk::DescriptorType::eStorageImage);
		writes[1].setImageInfo(imageInfo);

		// Update
		m_device->updateDescriptorSets(writes, nullptr);
	}


	void SimpleRaytracer::OffscreenRender()
	{
		SKHOLE_UNIMPL("OffscreenRender");
	}

	RendererData SimpleRaytracer::GetRendererData()
	{
		RendererData data;
		data.rendererName = "Simple Raytracer";
		data.materials.materialParameters = m_matParams;

		return data;
	}

	void SimpleRaytracer::InitVulkan()
	{
		SKHOLE_UNIMPL("InitVulkan");
	}

	ShrPtr<RendererDefinisionMaterial> SimpleRaytracer::GetMaterialDefinision() 
	{
		ShrPtr<RendererDefinisionMaterial> materialDef = MakeShr<RendererDefinisionMaterial>();
		materialDef->materialParameters = m_matParams;
		return materialDef;
	}

	ShrPtr<RendererDefinisionMaterial> SimpleRaytracer::GetMaterial(const ShrPtr<BasicMaterial>& material) 
	{
		ShrPtr<RendererDefinisionMaterial> materialDef = MakeShr<RendererDefinisionMaterial>();
		materialDef->materialName = material->materialName;

		materialDef->materialParameters = m_matParams; // Copy

		materialDef->materialParameters[0]->setParamValue(material->basecolor); // BaseColor
		materialDef->materialParameters[1]->setParamValue(material->metallic); // Metallic
		materialDef->materialParameters[2]->setParamValue(material->roughness); // Roughness
		
		return materialDef;	
	}


	//void SimpleRaytracer::DefineMaterial()
	//{
	//	m_matParams.resize(12);
	//	m_matParams[0] = MakeShr<MatParamBool>("bool", false);
	//}

}
