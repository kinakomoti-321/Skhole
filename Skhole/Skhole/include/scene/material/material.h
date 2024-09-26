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
		int BaseColorMap;

		float anisotropic = 0.0f;
		float metallic;
		int MetallicMap;

		float roughness;
		int RoughnessMap;

		float sheen;
		float clearcoat;

		int NormalMap;
		int HeightMap;

		vec4 emissionColor;
		float emissionIntensity;
		int EmissiveMap;

		float ior;
		float transmission;
	};

};
