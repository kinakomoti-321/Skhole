#pragma once

#include <include.h>
#include <common/log.h>
#include <common/math.h>
#include <scene/animation/animation.h>

namespace Skhole {

	enum ObjectType {
		INSTANCE,
		LIGHT,
		VOLUME
	};

	class Object {

	public:
		Object() {};
		~Object() {};

		virtual ObjectType GetObjectType() = 0;

		void SetAnimationKey();

		mat4 SetWorldTransformMatrix(float time);
		mat4 GetWorldTransformMatrix(float time);

		mat4 GetTransformMatrix(float time);

		vec3 GetTranslation(float time);
		Quaternion GetRotation(float time);
		vec3 GetScale(float time);

		void ResetWorldTransformMatrix();

		bool IsRoot();
		bool IsLear();

		const char* GetObjectName() { return objectName.c_str(); }

		bool haveParent() { return parentObject != nullptr; }
		bool haveChild() { return childObject != nullptr; }


	public:
		std::string objectName;
		ShrPtr<Object> parentObject = nullptr;
		ShrPtr<Object> childObject = nullptr;

		std::optional<mat4> worldTransformMatrix;

		bool useAnimation = false;

		vec3 localTranslation = vec3(0.0);
		Quaternion localQuaternion = Quaternion(0.0, 0.0, 0.0, 1.0);
		vec3 localScale = vec3(1.0);

		Animation<vec3> translationAnimation;
		Animation<Quaternion> rotationAnimation;
		Animation<vec3> scaleAnimation;
	};
};
