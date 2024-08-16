#pragma once

#include <include.h>
#include <scene/parameter/parameter.h>


namespace Skhole
{
	struct RendererDefinisionCamera {
		std::vector<ShrPtr<Parameter>> cameraParameters;
		std::string cameraName = "Camera";
	};

	struct BasicCamera {
		std::string cameraName = "Camera";

		vec3 position = vec3(0.0);
		vec3 cameraDir = vec3(0.0, 0.0, -1.0);
		vec3 cameraUp = vec3(0.0, 1.0, 0.0);

		float fov = 45.0f;
	};

}
