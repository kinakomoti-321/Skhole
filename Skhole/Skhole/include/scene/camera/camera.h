#pragma once

#include <include.h>
#include <scene/parameter/parameter.h>


namespace Skhole
{
	struct BasicCamera {
		std::string cameraName = "Camera";

		vec3 position = vec3(0.0);
		vec3 cameraDir = vec3(0.0, 0.0, -1.0);
		vec3 cameraUp = vec3(0.0, 1.0, 0.0);
		vec3 cameraRight = vec3(1.0, 0.0, 0.0);	

		float fov = 45.0f;
	};

	struct RendererDefinisionCamera {
		BasicCamera basicParameter;
		std::vector<ShrPtr<Parameter>> extensionParameters;
		std::string cameraName = "Camera";
	};


}
