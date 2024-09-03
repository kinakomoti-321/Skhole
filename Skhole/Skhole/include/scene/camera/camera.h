#pragma once

#include <include.h>
#include <scene/object/cameraObject.h>
#include <scene/parameter/parameter.h>


namespace Skhole
{
	struct RendererDefinisionCamera {
		vec3 position = vec3(0.0, 0.0, 10.0);
		vec3 foward = vec3(0.0, 0.0, -1.0);
		vec3 up = vec3(0.0, 1.0, 0.0);
		vec3 right = vec3(1.0, 0.0, 0.0);

		float fov = 45.0f;

		std::shared_ptr<CameraObject> camera = nullptr;

		std::vector<ShrPtr<Parameter>> extensionParameters;
		std::string cameraName = "Camera";


		vec3 GetCameraPosition(float time) {
			if (camera == nullptr) {
				return position;
			}
			else {
				return camera->GetCameraPosition(time);
			}
		}

		void GetCameraDirections(float time, vec3& camFoward, vec3& camUp, vec3& camRight) {
			if (camera == nullptr) {
				camFoward = this->foward;
				camUp = this->up;
				camRight = this->right;
			}
			else {
				camera->GetCameraDirection(time, camFoward, camUp, camRight);
			}
		}

		float GetYFov() {
			if (camera == nullptr) {
				return fov;
			}
			else {
				return camera->yFov;
			}
		}

	};


}
