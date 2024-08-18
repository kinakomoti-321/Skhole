#include <renderer/simple_raytracer.h>
#include <common/log.h>
#include <vulkan_helpler/vkutils.hpp>

namespace Skhole {

	//--------------------------------------
	// Interface Method
	//--------------------------------------

	SimpleRaytracer::SimpleRaytracer()
	{

	}

	SimpleRaytracer::~SimpleRaytracer()
	{

	}

	void SimpleRaytracer::InitImGui()
	{
		m_imGuiManager.Init(
			m_desc.window,
			*m_context.instance,
			m_context.physicalDevice,
			*m_context.device,
			m_context.queueIndex,
			m_context.queue,
			*m_imGuiRenderPass,
			2,
			m_swapchainContext.swapchainImages.size()
		);

	}

	void SimpleRaytracer::Init(RendererDesc& desc)
	{
		SKHOLE_LOG_SECTION("Initialze Renderer");
		m_desc = desc;

		//--------------------------------------
		// Vulkan Initialize
		//--------------------------------------
		VkHelper::Context::VulkanInitialzeInfo initInfo{};
		initInfo.apiVersion = VK_API_VERSION_1_2;
		initInfo.layers = m_layer;
		initInfo.extensions = m_extension;
		initInfo.useWindow = desc.useWindow;
		initInfo.window = desc.window;

		m_context.InitCore(initInfo);

		m_commandPool = vkutils::createCommandPool(*m_context.device, m_context.queueIndex);
		m_commandBuffer = vkutils::createCommandBuffer(*m_context.device, *m_commandPool);

		m_imGuiRenderPass = VkHelper::CreateRenderPass(
			vk::Format::eB8G8R8A8Unorm,
			vk::ImageLayout::eGeneral,
			vk::ImageLayout::ePresentSrcKHR,
			*m_context.device
		);

		//--------------------------------------
		// swapChain Initialize
		//--------------------------------------
		VkHelper::SwapChainInfo swapchainInfo{};
		swapchainInfo.physicalDevice = m_context.physicalDevice;
		swapchainInfo.device = *m_context.device;
		swapchainInfo.surface = *m_context.surface;
		swapchainInfo.queueIndex = m_context.queueIndex;
		swapchainInfo.queue = m_context.queue;
		swapchainInfo.commandPool = *m_commandPool;
		swapchainInfo.renderPass = m_imGuiRenderPass.get();

		swapchainInfo.swapcahinImageUsage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment;

		swapchainInfo.width = desc.Width;
		swapchainInfo.height = desc.Height;

		m_swapchainContext.Init(swapchainInfo);

		//--------------------------------------
		// Create Pipeline
		//--------------------------------------

		CreateRaytracingPipeline();

		InitImGui();

		//--------------------------------------
		// Create Buffer
		//--------------------------------------
		uniformBufferObject.frame = 0;
		uniformBufferObject.spp = 100;
		uniformBufferObject.width = m_desc.Width;
		uniformBufferObject.height = m_desc.Height;

		uniformBufferObject.cameraDir = vec3(0.0f, 0.0f, -1.0f);
		uniformBufferObject.cameraPos = vec3(0.0, 30.0, 50.0);
		uniformBufferObject.cameraUp = vec3(0.0f, 1.0f, 0.0f);
		uniformBufferObject.cameraRight = vec3(1.0f, 0.0f, 0.0f);

		m_uniformBuffer.init(m_context.physicalDevice, *m_context.device, sizeof(UniformBufferObject),
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			&uniformBufferObject
		);

		SKHOLE_LOG_SECTION("Initialze Renderer Completed");
	}

	void SimpleRaytracer::DestroyScene() {

	}

	void SimpleRaytracer::UpdateScene(const UpdateCommand& command) {

	}

	void SimpleRaytracer::InitFrameGUI() {
		m_imGuiManager.NewFrame();
	}

	void SimpleRaytracer::Destroy()
	{
		m_context.device->waitIdle();

		m_bindingManager.Release(*m_context.device);
		m_imGuiManager.Destroy(*m_context.device);
	}

	void SimpleRaytracer::Resize(unsigned int width, unsigned int height)
	{
		SKHOLE_UNIMPL("Resize");
	}

	void SimpleRaytracer::Render(const RenderInfo& renderInfo)
	{

		float frame = 0;

		FrameStart(frame);

		vk::UniqueSemaphore imageAvailableSemaphore =
			m_context.device->createSemaphoreUnique({});

		auto& swapchain = m_swapchainContext.swapchain;
		auto& swapchainImages = m_swapchainContext.swapchainImages;
		auto& swapchainImageViews = m_swapchainContext.swapchainImageViews;
		auto& swapchainFramebuffers = m_swapchainContext.frameBuffers;

		auto result = m_context.device->acquireNextImageKHR(
			*swapchain, std::numeric_limits<uint64_t>::max(),
			*imageAvailableSemaphore);
		if (result.result != vk::Result::eSuccess) {
			std::cerr << "Failed to acquire next image.\n";
			std::abort();
		}

		uint32_t imageIndex = result.value;
		UpdateDescriptorSet(*swapchainImageViews[imageIndex]);

		RecordCommandBuffer(swapchainImages[imageIndex], *swapchainFramebuffers[imageIndex]);

		vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eTopOfPipe };
		vk::SubmitInfo submitInfo{};
		submitInfo.setWaitDstStageMask(waitStage);
		submitInfo.setCommandBuffers(*m_commandBuffer);
		submitInfo.setWaitSemaphores(*imageAvailableSemaphore);
		m_context.queue.submit(submitInfo);

		m_context.queue.waitIdle();

		vk::PresentInfoKHR presentInfo{};
		presentInfo.setSwapchains(*swapchain);
		presentInfo.setImageIndices(imageIndex);
		if (m_context.queue.presentKHR(presentInfo) != vk::Result::eSuccess) {
			std::cerr << "Failed to present.\n";
			std::abort();
		}

		frame++;

		FrameEnd();
	}

	void SimpleRaytracer::SetScene(ShrPtr<Scene> scene) {
		m_scene = scene;
		InitBufferManager();
		InitAccelerationStructures();
	}

	void SimpleRaytracer::InitBufferManager() {
		SKHOLE_LOG("Init Buffer Manager");

		m_sceneBufferManager.SetScene(m_scene);

		m_sceneBufferManager.InitGeometryBuffer(m_context.physicalDevice, *m_context.device);
		m_sceneBufferManager.InitInstanceBuffer(m_context.physicalDevice, *m_context.device);
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

	ShrPtr<RendererDefinisionCamera> SimpleRaytracer::GetCamera(const ShrPtr<BasicCamera>& camera)
	{
		ShrPtr<RendererDefinisionCamera> cameraDef = MakeShr<RendererDefinisionCamera>();
		cameraDef->cameraName = camera->cameraName;

		cameraDef->basicParameter.position = camera->position;
		cameraDef->basicParameter.cameraDir = camera->cameraDir;
		cameraDef->basicParameter.cameraUp = camera->cameraUp;
		cameraDef->basicParameter.fov = camera->fov;

		cameraDef->extensionParameters = m_camExtensionParams;

		return cameraDef;
	}

	//--------------------------------------
	// Internal Method
	//--------------------------------------
	void SimpleRaytracer::CreateRaytracingPipeline() {
		PrepareShader();

		CreateDescSetLayout();
		CreateDescriptorPool();
		CreateDescSet();

		CreatePipeline();
		CreateShaderBindingTable();
	}
	
	void SimpleRaytracer::CreateDescriptorPool()
	{
		m_bindingManager.SetPool(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, *m_context.device);
	}

	void SimpleRaytracer::CreateDescSetLayout() {

		m_bindingManager.bindings = {
			{0, vk::DescriptorType::eAccelerationStructureKHR, 1, vk::ShaderStageFlagBits::eRaygenKHR},
			{1, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eRaygenKHR},
			{2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eRaygenKHR },
			{3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},
			{4, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR}
		};

		m_bindingManager.SetLayout(*m_context.device);
	}

	void SimpleRaytracer::FrameStart(float frame) {

		auto& camera = m_scene->m_camera;
		uniformBufferObject.cameraPos = camera->basicParameter.position;
		uniformBufferObject.cameraDir = camera->basicParameter.cameraDir;
		uniformBufferObject.cameraUp = camera->basicParameter.cameraUp;
		uniformBufferObject.cameraRight = camera->basicParameter.cameraRight;
		uniformBufferObject.cameraParam.x = camera->basicParameter.fov;
		uniformBufferObject.cameraParam.y = static_cast<float>(m_desc.Width) / static_cast<float>(m_desc.Height);

		void* map = m_uniformBuffer.Map(*m_context.device, 0, sizeof(UniformBufferObject));
		memcpy(map, &uniformBufferObject, sizeof(UniformBufferObject));
		m_uniformBuffer.Ummap(*m_context.device);

		m_scene->SetTransformMatrix(frame);

		m_sceneBufferManager.FrameUpdateInstance(*m_context.device, frame);
		m_asManager.BuildTLAS(m_sceneBufferManager, m_context.physicalDevice, *m_context.device, *m_commandPool, m_context.queue);
	}

	void SimpleRaytracer::FrameEnd()
	{
		m_asManager.ReleaseTLAS(*m_context.device);
	}

	void SimpleRaytracer::RecordCommandBuffer(vk::Image image, vk::Framebuffer frameBuffer) {
		m_commandBuffer->begin(vk::CommandBufferBeginInfo{});
		vkutils::setImageLayout(*m_commandBuffer, image, vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eGeneral);

		m_commandBuffer->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, *m_pipeline);

		m_commandBuffer->bindDescriptorSets(
			vk::PipelineBindPoint::eRayTracingKHR,
			*m_pipelineLayout,
			0,
			m_bindingManager.descriptorSet,
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

		VkHelper::BindingManager::WritingInfo info;
		info.numAS = 1;
		info.numImage = 1;
		info.numBuffer = 3;
		m_bindingManager.StartWriting(info);

		m_bindingManager.WriteAS(
			*m_asManager.TLAS.accel, 0, 1, *m_context.device
		);

		m_bindingManager.WriteImage(
			imageView, vk::ImageLayout::eGeneral, VK_NULL_HANDLE,
			vk::DescriptorType::eStorageImage, 1, 1, *m_context.device
		);

		m_bindingManager.WriteBuffer(
			*m_uniformBuffer.buffer, 0, sizeof(UniformBufferObject),
			vk::DescriptorType::eUniformBuffer, 2, 1, *m_context.device
		);

		//m_bindingManager.WriteBuffer(
		//	*m_sceneBufferManager.vertexBuffer.buffer, 0, , 
		//	vk::DescriptorType::eStorageBuffer, 3, 1, *m_context.device
		//);

		m_bindingManager.WriteBuffer(
			*m_sceneBufferManager.vertexBuffer.buffer, 0,m_sceneBufferManager.vertexBufferSize, 
			vk::DescriptorType::eStorageBuffer, 3, 1, *m_context.device
		);

		m_bindingManager.EndWriting(*m_context.device);
	}

	RendererData SimpleRaytracer::GetRendererData()
	{
		RendererData data;
		data.rendererName = "Simple Raytracer";
		data.materials.materialParameters = m_matParams;

		return data;
	}

	void SimpleRaytracer::InitAccelerationStructures() {
		m_asManager.BuildBLAS(m_sceneBufferManager, m_context.physicalDevice, *m_context.device, *m_commandPool, m_context.queue);
	}

#define SHADER_FILE_PATH "shader/"
	void SimpleRaytracer::AddShader(
		uint32_t shaderIndex,
		const std::string& shaderName,
		vk::ShaderStageFlagBits stage
	)
	{
		shaderModules[shaderIndex] = vkutils::createShaderModule(*m_context.device, SHADER_FILE_PATH + shaderName);
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


	void SimpleRaytracer::CreateDescSet() {
		std::cout << "Create desc set\n";

		m_bindingManager.SetDescriptorSet(*m_context.device);

	}

	void SimpleRaytracer::CreatePipeline() {
		std::cout << "Create pipeline\n";

		// Create pipeline layout
		vk::PipelineLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.setSetLayouts(m_bindingManager.descriptorSetLayout);
		m_pipelineLayout = m_context.device->createPipelineLayoutUnique(layoutCreateInfo);

		// Create pipeline
		vk::RayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
		pipelineCreateInfo.setLayout(*m_pipelineLayout);
		pipelineCreateInfo.setStages(shaderStages);
		pipelineCreateInfo.setGroups(shaderGroups);
		pipelineCreateInfo.setMaxPipelineRayRecursionDepth(1);
		auto result = m_context.device->createRayTracingPipelineKHRUnique(
			nullptr, nullptr, pipelineCreateInfo);
		if (result.result != vk::Result::eSuccess) {
			std::cerr << "Failed to create ray tracing pipeline.\n";
			std::abort();
		}
		m_pipeline = std::move(result.value);
	}

	void SimpleRaytracer::CreateShaderBindingTable() {
		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties =
			vkutils::getRayTracingProps(m_context.physicalDevice);
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
		sbt.init(m_context.physicalDevice, *m_context.device, sbtSize,
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

		auto result = m_context.device->getRayTracingShaderGroupHandlesKHR(
			*m_pipeline, 0, handleCount, handleStorageSize, handleStorage.data());
		if (result != vk::Result::eSuccess) {
			std::cerr << "Failed to get ray tracing shader group handles.\n";
			std::abort();
		}

		// Copy handles
		uint32_t handleIndex = 0;
		uint8_t* sbtHead =
			static_cast<uint8_t*>(m_context.device->mapMemory(*sbt.memory, 0, sbtSize));

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

}
