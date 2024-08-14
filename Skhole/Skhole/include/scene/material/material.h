#pragma once
#include <include.h>
#include <scene/material/texture.h>
#include <common/log.h>

using namespace VectorLikeGLSL;

namespace Skhole {
	enum MaterialParameterType
	{
		FLOAT,
		COLOR,
		BOOL,
		TEXTURE,
	};

	struct MaterialParameter{
		MaterialParameterType type;
		std::string paramName;
		
		float paramFloat;
		vec4 paramColor;
		bool paramBool;
		Texture paramMatTex;

		std::string GetMaterialParamInfo() {
			std::string paramString = "";
			paramString += "[Name] " + paramName + " [Type] ";
			switch (type)
			{
			case Skhole::FLOAT:
				paramString = "FLOAT";
				break;
			case Skhole::COLOR:
				paramString = "COLOR";
				break;
			case Skhole::BOOL:
				paramString = "BOOL";
				break;
			case Skhole::TEXTURE:
				paramString = "TEXTURE";
				break;
			default:
				SKHOLE_UNIMPL("Material Type");
				break;
			}

			return paramString;
		}
	};

	struct RendererMaterialParameter {
		std::vector<MaterialParameter> material_parameters;

		std::string GetMaterialInfoString() {
			std::string materialInfoString = "";

			for (auto& param : material_parameters) {
				materialInfoString += param.GetMaterialParamInfo() + "\n";
			}

			return materialInfoString;
		}
	};

	struct BasicalMaterial {
		vec4 basecolor;
		Texture BaseColorMap;

		float metallic;
		Texture MetallicMap;

		float roughness;
		Texture RoughnessMap;

		Texture NormalMap;
		Texture HeightMap;

		vec3 emissiveColor;
		Texture EmissiveMap;

		float ior;
		float transmission;
	};

};
