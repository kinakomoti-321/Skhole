#pragma once

#include <renderer/renderer.h>

#include <renderer/common/buffer_manager.h>

#include <vulkan_helpler/vkutils.hpp>
#include <vulkan_helpler/vk_buffer.h>
#include <vulkan_helpler/vk_hepler.h>
#include <vulkan_helpler/vk_imgui.h>

#include <post_process/post_processor.h>
#include <post_process/post_processor_interface.h>

#include <common/util.h>

#include <scene/scene.h>

namespace Skhole
{
	class SimpleRaytracer : public Renderer
	{
	public:
		//--------------------------------------
		// Interface Method
		//--------------------------------------
		SimpleRaytracer();
		~SimpleRaytracer();

		void InitializeCore(const RendererDesc& desc) override;
		void ResizeCore(unsigned int width, unsigned int height) override;
		void DestroyCore() override;

		void SetScene(ShrPtr<Scene> scene) override;
		void DestroyScene() override;

		void UpdateScene(const UpdataInfo& command) override;

		void InitFrameGUI() override;
		void RealTimeRender(const RealTimeRenderingInfo& renderInfo) override;
		void OfflineRender(const OfflineRenderingInfo& renderInfo) override;

		std::vector<const char*> GetLayer() override {
			return m_layer;
		}
		std::vector<const char*> GetExtensions() override {
			return m_extension;
		}

		ShaderPaths GetShaderPaths() override {
			ShaderPaths paths;
			paths.raygen = "shader/simple_raytracer/raygen.rgen.spv";
			paths.miss = "shader/simple_raytracer/miss.rmiss.spv";
			paths.closestHit = "shader/simple_raytracer/closesthit.rchit.spv";
			return paths;
		}

		// Material ------------------------------------------
		struct Material {
			vec4 baseColor;
			float roughness;
			float metallic;

			float emissionIntesity = 1.0;
			float padding = 0.0;

			vec4 emissionColor;
		};

		const std::vector<ShrPtr<Parameter>> m_matParams =
		{
			MakeShr<ParamCol>("BaseColor", vec4(0.8f)),
			MakeShr<ParamFloat>("Roughness", 0.0f),
			MakeShr<ParamFloat>("Metallic", 0.0f),

			MakeShr<ParamFloat>("EmissionIntensity", 0.0f),
			MakeShr<ParamCol>("Emission", vec4(0.0)),
		};

		void DefineMaterial(ShrPtr<RendererDefinisionMaterial>& materialDef, const ShrPtr<BasicMaterial>& material) override
		{
			CopyParameter(m_matParams, materialDef->materialParameters);

			materialDef->materialParameters[0]->setParamValue(material->basecolor);
			materialDef->materialParameters[1]->setParamValue(material->metallic);
			materialDef->materialParameters[2]->setParamValue(material->roughness);
			materialDef->materialParameters[3]->setParamValue(material->emissionIntensity);
			materialDef->materialParameters[4]->setParamValue(material->emissionColor);
		}

		Material ConvertMaterial(const ShrPtr<RendererDefinisionMaterial>& materialDef) {
			Material material;

			material.baseColor = GetParamColValue(materialDef->materialParameters[0]);
			material.metallic = GetParamFloatValue(materialDef->materialParameters[1]);
			material.roughness = GetParamFloatValue(materialDef->materialParameters[2]);
			material.emissionIntesity = GetParamFloatValue(materialDef->materialParameters[3]);
			material.emissionColor = GetParamColValue(materialDef->materialParameters[4]);

			return material;
		}

		// Camera ------------------------------------------
		const std::vector<ShrPtr<Parameter>> m_camExtensionParams =
		{
			MakeShr<ParamVec>("LookPoint",vec3(0.0f)),
		};

		void DefineCamera(const ShrPtr<RendererDefinisionCamera>& cameraDef) override
		{
			CopyParameter(m_camExtensionParams, cameraDef->extensionParameters);
		}

		// Renderer ------------------------------------------
		const std::vector<ShrPtr<Parameter>> m_rendererExtensionParams =
		{
			MakeShr<ParamUint>("Ckeck Mode",0),
		};

		ShrPtr<RendererParameter> GetRendererParameter() override {
			ShrPtr<RendererParameter> rendererParameter = MakeShr<RendererParameter>();
			rendererParameter->rendererName = "Simple Raytracer";
			rendererParameter->frame = 0;
			rendererParameter->maxSPP = 100;
			rendererParameter->numSPP = 1;

			CopyParameter(m_rendererExtensionParams, rendererParameter->rendererParameters);
			rendererParameter->posproParameters = m_postProcessor->GetParamter();

			return rendererParameter;
		}

	private:
		struct Uniform {
			uint32_t spp;
			uint32_t frame;
			uint32_t sample;
			uint32_t mode;

			uint32_t width;
			uint32_t height;

			uint32_t padding[2];

			vec3 cameraPos;
			vec3 cameraDir;
			vec3 cameraUp;
			vec3 cameraRight;
			vec4 cameraParam;
		};

	private:
		void FrameStart(float time);
		void FrameEnd();

		void UpdateMaterialBuffer(uint32_t matId)
		{
			auto material = ConvertMaterial(m_scene->m_materials[matId]);
			m_materialBuffer.SetMaterial(material, matId);

			m_materialBuffer.UpdateBufferIndex(matId, *m_context.device, *m_commandPool, m_context.queue);
		}

		void InitializeBiniding() override {
			std::vector<VkHelper::BindingLayoutElement> bindingLayout = {
				{0, vk::DescriptorType::eAccelerationStructureKHR, 1, vk::ShaderStageFlagBits::eRaygenKHR},
				{1, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eRaygenKHR},
				{2, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eRaygenKHR},
				{3, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eRaygenKHR },
				{4, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},
				{5, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},
				{6, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},
				{7, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},
				{8, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},
				{9, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},
			};

			m_bindingManager.SetBindingLayout(*m_context.device, bindingLayout, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
		}

		void UpdateDescriptorSet() {
			auto& accumImage = m_renderImages.GetAccumImage();
			auto& renderImage = m_renderImages.GetRenderImage();
			auto& posproIamge = m_renderImages.GetPostProcessedImage();

			m_bindingManager.StartWriting();

			m_bindingManager.WriteAS(
				*m_asManager.TLAS.accel, 0, 1, *m_context.device
			);

			m_bindingManager.WriteImage(
				renderImage.GetImageView(), vk::ImageLayout::eGeneral, VK_NULL_HANDLE,
				vk::DescriptorType::eStorageImage, 1, 1, *m_context.device
			);

			m_bindingManager.WriteImage(
				accumImage.GetImageView(), vk::ImageLayout::eGeneral, VK_NULL_HANDLE,
				vk::DescriptorType::eStorageImage, 2, 1, *m_context.device
			);

			m_bindingManager.WriteBuffer(
				m_uniformBuffer.GetBuffer(), 0, m_uniformBuffer.GetBufferSize(),
				vk::DescriptorType::eUniformBuffer, 3, 1, *m_context.device
			);

			m_bindingManager.WriteBuffer(
				m_sceneBufferManager.vertexBuffer.GetDeviceBuffer(), 0, m_sceneBufferManager.vertexBuffer.GetBufferSize(),
				vk::DescriptorType::eStorageBuffer, 4, 1, *m_context.device
			);

			m_bindingManager.WriteBuffer(
				m_sceneBufferManager.indexBuffer.GetDeviceBuffer(), 0, m_sceneBufferManager.indexBuffer.GetBufferSize(),
				vk::DescriptorType::eStorageBuffer, 5, 1, *m_context.device
			);

			m_bindingManager.WriteBuffer(
				m_sceneBufferManager.geometryBuffer.GetDeviceBuffer(), 0, m_sceneBufferManager.geometryBuffer.GetBufferSize(),
				vk::DescriptorType::eStorageBuffer, 6, 1, *m_context.device
			);

			m_bindingManager.WriteBuffer(
				m_sceneBufferManager.instanceBuffer.GetDeviceBuffer(), 0, m_sceneBufferManager.instanceBuffer.GetBufferSize(),
				vk::DescriptorType::eStorageBuffer, 7, 1, *m_context.device
			);

			m_bindingManager.WriteBuffer(
				m_materialBuffer.GetBuffer(), 0, m_materialBuffer.GetBufferSize(),
				vk::DescriptorType::eStorageBuffer, 8, 1, *m_context.device
			);

			m_bindingManager.WriteBuffer(
				m_sceneBufferManager.matIndexBuffer.GetDeviceBuffer(), 0, m_sceneBufferManager.matIndexBuffer.GetBufferSize(),
				vk::DescriptorType::eStorageBuffer, 9, 1, *m_context.device
			);


			m_bindingManager.EndWriting(*m_context.device);
		}

	private:
		//--------------------------------------
		// Renderer Data
		//--------------------------------------
		std::string rendererName = "Simple RayTracer";

		std::vector<const char*> m_layer = {
			"VK_LAYER_KHRONOS_validation"
		};

		std::vector<const char* > m_extension = {
			// For swapchain
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,

			// For ray tracing
			VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,

			// For GLSL
			VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME,
			VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
		};

		SceneBufferaManager m_sceneBufferManager;
		ASManager m_asManager;

		MaterialBuffer<Material> m_materialBuffer;
		UniformBuffer<Uniform> m_uniformBuffer;
	};
}
