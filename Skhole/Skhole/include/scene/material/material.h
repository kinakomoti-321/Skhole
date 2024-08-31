#pragma once
#include <include.h>
#include <scene/material/texture.h>
#include <scene/parameter/parameter.h>

#include <common/log.h>

using namespace VectorLikeGLSL;

namespace Skhole {

	struct RendererDefinisionMaterial {
		std::vector<ShrPtr<Parameter>> materialParameters;
		std::string materialName = "DefaultMaterial";
	};


	struct BasicMaterial {
		std::string materialName = "DefaultMaterial";

		vec4 basecolor;
		Texture BaseColorMap;

		float metallic;
		Texture MetallicMap;

		float roughness;
		Texture RoughnessMap;

		float sheen;
		float clearcoat;

		Texture NormalMap;
		Texture HeightMap;

		vec4 emissionColor;
		float emissionIntensity;
		Texture EmissiveMap;

		float ior;
		float transmission;
	};

};
