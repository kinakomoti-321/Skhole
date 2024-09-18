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
			materialDef->materialParameters[1]->setParamValue(material->metallic);
			materialDef->materialParameters[2]->setParamValue(material->roughness);
			materialDef->materialParameters[3]->setParamValue(material->emissionIntensity);
			materialDef->materialParameters[4]->setParamValue(material->emissionColor);
		}

		void DefineCamera(const ShrPtr<RendererDefinisionCamera>& cameraDef) override
		{
			CopyParameter(camExtension, cameraDef->extensionParameters);
		}

		ShrPtr<RendererParameter> GetRendererParameter() override
		{
			auto rendererParameter = MakeShr<RendererParameter>();
			rendererParameter->rendererName = "Simple Raytracer";
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

	private:
		// Buffers
		SceneBufferaManager m_sceneBufferManager;
		ASManager m_asManager;

		MaterialBuffer<Material> m_materialBuffer;
		UniformBuffer<Uniform> m_uniformBuffer;
	};
}

