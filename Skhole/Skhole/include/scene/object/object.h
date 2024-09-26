#pragma once

#include <include.h>
#include <common/log.h>
#include <common/math.h>
#include <scene/animation/animation.h>

namespace Skhole {

	typedef enum class ObjectType {
		INSTANCE,
		LIGHT,
		VOLUME,
		CAMERA,
	};

	static std::string ObjectType2Name(ObjectType objectType) {
		std::string name;
		switch (objectType) {
		case ObjectType::INSTANCE:
			name = "INSTANCE";
			break;
		case ObjectType::LIGHT:
			name = "LIGHT";
			break;
		case ObjectType::VOLUME:
			name = "VOLUME";
			break;
		case ObjectType::CAMERA:
			name = "CAMERA";
			break;
		default:
			SKHOLE_UNIMPL();
			break;
		}
		return name;
	}

	static ObjectType ObjectName2Type(const std::string& name) {
		ObjectType objectType;
		if (name == "INSTANCE") {
			objectType = ObjectType::INSTANCE;
		}
		else if (name == "LIGHT") {
			objectType = ObjectType::LIGHT;
		}
		else if (name == "VOLUME") {
			objectType = ObjectType::VOLUME;
		}
		else if (name == "CAMERA") {
			objectType = ObjectType::CAMERA;
		}
		else {
			SKHOLE_UNIMPL();
		}
		return objectType;
	}



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
