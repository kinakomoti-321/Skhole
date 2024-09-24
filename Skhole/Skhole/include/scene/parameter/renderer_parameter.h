#pragma once
#include <include.h>
#include <post_process/post_processor.h>
#include <scene/parameter/parameter.h>

namespace Skhole {
	struct RendererParameter {
		RendererParameter() {}
		~RendererParameter() {}

		std::string rendererName;
		uint32_t maxSPP;
		uint32_t numSPP;
		uint32_t sppPerFrame = 1;
		uint32_t frame;

		std::vector<ShrPtr<Parameter>> rendererParameters;
		PostProcessParameter posproParameters;
	};
};

