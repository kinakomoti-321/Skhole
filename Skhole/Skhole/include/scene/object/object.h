#pragma once

#include <include.h>
#include <common/log.h>
#include <common/math.h>

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

		mat4 SetWorldTransformMatrix(float frame);
		mat4 GetWorldTransformMatrix(float frame);

		mat4 GetLocalTransformMatrix(float frame);

		vec3 GetLocalPosition(float frame);
		vec3 GetLocalRotationEular(float frame);	
		vec3 GetLocalScale(float frame);

		void ResetWorldTransformMatrix();
		
		bool IsRoot();
		bool IsLear();
		
		std::string GetObjectName(){return objectName;}

	public:
		std::string objectName;
		ShrPtr<Object> parentObject = nullptr;	
		ShrPtr<Object> childObject = nullptr;

		std::optional<mat4> worldTransformMatrix;

		bool useAnimation = false;

		vec3 localPosition;
		vec3 localRotationEular;
		vec3 localScale;

	};
};
