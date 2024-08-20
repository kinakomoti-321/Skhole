#pragma once
#include <include.h>
#include <scene/parameter/parameter.h>

namespace Skhole {
	struct RendererParameter {
		RendererParameter(){}
		~RendererParameter(){}

		std::string rendererName;
		uint32_t spp;
		uint32_t sample;
		uint32_t frame;

		std::vector<ShrPtr<Parameter>> rendererParameters;
	};
};

