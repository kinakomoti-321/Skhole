#pragma once
#include <include.h>
#include <scene/object/object.h>

using namespace VectorLikeGLSL;
namespace Skhole {

	class CameraObject :public Object {
	public:
		CameraObject() {};
		~CameraObject() {};

		ObjectType GetObjectType() override { return ObjectType::CAMERA; };

		float yFov = 45.0f;

		vec3 GetCameraPosition(float time) {
			mat4 worldTransform = GetWorldTransformMatrix(time);
			vec4 worldPos = worldTransform * vec4(0.0, 0.0, 0.0, 1.0);
			return worldPos.xyz;
		}

		void GetCameraDirection(float time,vec3& foward, vec3& up, vec3& right) {
			mat4 worldTransform = GetWorldTransformMatrix(time);
			mat3 directionTransform = NormalTransformMatrix3x3(worldTransform);
			foward = directionTransform * vec3(0.0, 0.0, -1.0);	
			up = directionTransform * vec3(0.0, 1.0, 0.0);
			right = directionTransform * vec3(1.0, 0.0, 0.0);
		}

		float GetYFov() { return yFov; }
	};

}
