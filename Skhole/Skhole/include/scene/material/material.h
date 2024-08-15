#pragma once
#include <include.h>
#include <scene/material/texture.h>
#include <common/log.h>

using namespace VectorLikeGLSL;

namespace Skhole {
	enum MaterialParameterType
	{
		FLOAT,
		VECTOR,
		COLOR,
		BOOL,
		TEXTURE,
	};

	struct MaterialParameterBool {
		MaterialParameterBool(std::string name, bool value) : value(value), name(name) {}

		MaterialParameterType type = MaterialParameterType::BOOL;
		bool value;
		std::string name;
	};

	struct MaterialParameterFloat {
		MaterialParameterFloat(std::string name, float value) : value(value), name(name) {}
		float& Value(){return value;}

		MaterialParameterType type = MaterialParameterType::FLOAT;
		float value;
		std::string name;
	};
	struct MaterialParameterVector {
		MaterialParameterVector(std::string name, vec3 value) : value(value), name(name) {}
		vec3& Value() { return value; }

		MaterialParameterType type = MaterialParameterType::VECTOR;
		vec3 value;
		std::string name;
	};
	struct MaterialParameterColor {
		MaterialParameterColor(std::string name, vec4 value) : value(value), name(name) {}
		vec4& Value() { return value; }

		MaterialParameterType type = MaterialParameterType::COLOR;
		vec4 value;
		std::string name;
	};
	struct MaterialParameterTexture {
		MaterialParameterTexture(std::string name, Texture value) : value(value), name(name) {}
		Texture& Value() { return value; }

		MaterialParameterType type = MaterialParameterType::TEXTURE;
		Texture value;
		std::string name;
	};

	struct MaterialParameter {
		MaterialParameter(MaterialParameterBool value) : parameter(value) {}
		MaterialParameter(MaterialParameterFloat value) : parameter(value) {}
		MaterialParameter(MaterialParameterVector value) : parameter(value) {}
		MaterialParameter(MaterialParameterColor value) : parameter(value) {}
		MaterialParameter(MaterialParameterTexture value) : parameter(value) {}

		std::variant<
			MaterialParameterBool,
			MaterialParameterFloat,
			MaterialParameterVector,
			MaterialParameterColor,
			MaterialParameterTexture
		> parameter;
	};

	struct RendererDefinisionMaterial {
		std::vector<MaterialParameter> materialParameters;

		RendererDefinisionMaterial Copy()
		{
			RendererDefinisionMaterial copiedMaterial;
			copiedMaterial.materialParameters = materialParameters;
			return copiedMaterial;
		}
	};


	struct BasicMaterial {
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
