#pragma once

#include <renderer/renderer.h>

#include <renderer/common/buffer_manager.h>

#include <vulkan_helpler/vkutils.hpp>
#include <vulkan_helpler/vk_buffer.h>
#include <vulkan_helpler/vk_hepler.h>
#include <vulkan_helpler/vk_imgui.h>

#include <common/util.h>
#include <scene/scene.h>

namespace Skhole
{
	class VNDF_Renderer : public Renderer
	{
	public:
		VNDF_Renderer() {}
		~VNDF_Renderer() {}

		void InitializeCore(RendererDesc& desc) override;
		void ResizeCore(unsigned int width, unsigned int height) override;
		void DestroyCore() override;

		void SetScene(ShrPtr<Scene> scene) override;
		void DestroyScene() override;

		void DefineMaterial(ShrPtr<RendererDefinisionMaterial>& materialDef, const ShrPtr<BasicMaterial>& material) override
		{
			CopyParameter(matParams, materialDef->materialParameters);

			materialDef->materialParameters[0]->setParamValue(material->basecolor);
			materialDef->materialParameters[1]->setParamValue(0.0f);
			materialDef->materialParameters[2]->setParamValue(material->metallic);
			materialDef->materialParameters[3]->setParamValue(material->roughness);
			materialDef->materialParameters[4]->setParamValue(material->emissionIntensity);
			materialDef->materialParameters[5]->setParamValue(material->emissionColor);
		}

		void DefineCamera(const ShrPtr<RendererDefinisionCamera>& cameraDef) override
		{
			CopyParameter(camExtension, cameraDef->extensionParameters);
		}

		ShrPtr<RendererParameter> GetRendererParameter() override
		{
			auto rendererParameter = MakeShr<RendererParameter>();
			rendererParameter->rendererName = "VNDF Raytracer";
			rendererParameter->frame = 0;
			rendererParameter->spp = 100;
			rendererParameter->sample = 1;

			CopyParameter(rendererExtensions, rendererParameter->rendererParameters);
			rendererParameter->posproParameters = m_postProcessor->GetParamter();

			return rendererParameter;
		}

		std::vector<const char*> GetLayer() override {
			return m_layer;
		}
		std::vector<const char*> GetExtensions() override {
			return m_extension;
		}

		void InitFrameGUI() override;

		void UpdateScene(const UpdataInfo& command) override;

		void RealTimeRender(const RealTimeRenderingInfo& renderInfo) override;
		void OfflineRender(const OfflineRenderingInfo& renderInfo) override;

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

		void UpdateDescriptorSet()
		{
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

		ShaderPaths GetShaderPaths() override {
			ShaderPaths paths;
			paths.raygen = "shader/vndf_renderer/raygen.rgen.spv";
			paths.miss = "shader/vndf_renderer/miss.rmiss.spv";
			paths.closestHit = "shader/vndf_renderer/closesthit.rchit.spv";
			return paths;
		}

	private:
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

		struct Uniform {
			uint32_t spp;
			uint32_t frame;
			uint32_t sample;
			uint32_t mode;

			uint32_t width;
			uint32_t height;

			vec3 cameraPos;
			vec3 cameraDir;
			vec3 cameraUp;
			vec3 cameraRight;
			vec4 cameraParam;
		};

		const ParamDefine camExtension =
		{
		};

		const ParamDefine rendererExtensions =
		{
		};

		struct Material {
			vec4 baseColor;
			float anisotropic;
			float roughness;
			float metallic;

			float emissionIntesity = 1.0;
			vec4 emissionColor;
		};

		const ParamDefine matParams =
		{
			MakeShr<ParamCol>("BaseColor", vec4(0.8f)),
			MakeShr<ParamFloat>("Anistropic",0.0f),
			MakeShr<ParamFloat>("Roughness", 0.0f),
			MakeShr<ParamFloat>("Metallic", 0.0f),

			MakeShr<ParamFloat>("EmissionIntensity", 0.0f),
			MakeShr<ParamCol>("Emission", vec4(0.0)),
		};

	private:
		// Internal methods
		Material ConvertMaterial(const ShrPtr<RendererDefinisionMaterial>& material)
		{
			Material mat;
			mat.baseColor = GetParamColValue(material->materialParameters[0]);
			mat.anisotropic = GetParamFloatValue(material->materialParameters[1]);
			mat.roughness = GetParamFloatValue(material->materialParameters[2]);
			mat.metallic = GetParamFloatValue(material->materialParameters[3]);

			mat.emissionIntesity = GetParamFloatValue(material->materialParameters[4]);
			mat.emissionColor = GetParamColValue(material->materialParameters[5]);

			return mat;
		}

		void UpdateMaterialBuffer(uint32_t matId)
		{
			auto& device = *m_context.device;
			auto& commandPool = *m_commandPool;
			auto& queue = m_context.queue;

			auto material = ConvertMaterial(m_scene->m_materials[matId]);
			m_materialBuffer.SetMaterial(material, matId);
			m_materialBuffer.UpdateBuffer(device, commandPool, queue);
		}

	private:
		// Buffers
		SceneBufferaManager m_sceneBufferManager;
		ASManager m_asManager;

		MaterialBuffer<Material> m_materialBuffer;
		UniformBuffer<Uniform> m_uniformBuffer;
	};
}

