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

		vec4 basecolor = vec4(1.0);
		int BaseColorMap = -1;

		float anisotropic = 0.0f;
		float metallic = 0;
		int MetallicMap = -1;

		float roughness = 0;
		int RoughnessMap = -1;

		float sheen = 0;
		float clearcoat = 0;

		int NormalMap = -1;
		int HeightMap = -1;

		vec4 emissionColor = vec4(0.0);
		float emissionIntensity = 0.0;
		int EmissiveMap = -1;

		float ior = 1.0;
		float transmission = 0.0;
	};

};
