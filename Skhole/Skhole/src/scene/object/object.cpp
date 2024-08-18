#include <scene/object/object.h>

using namespace VectorLikeGLSL;
namespace Skhole {
	mat4 Object::SetWorldTransformMatrix(float frame) {
		
		if (worldTransformMatrix.has_value()) {
			return worldTransformMatrix.value();
		}
		
		if (IsRoot()) {
			worldTransformMatrix = GetLocalTransformMatrix(frame);
		}
		else {
			worldTransformMatrix = parentObject->SetWorldTransformMatrix(frame) * GetLocalTransformMatrix(frame);
		}
		
		return worldTransformMatrix.value();
	}

	mat4 Object::GetWorldTransformMatrix(float frame) {
		if (worldTransformMatrix.has_value()) {
			return worldTransformMatrix.value();
		}
		else {
			return SetWorldTransformMatrix(frame);
		}
	}

	mat4 Object::GetLocalTransformMatrix(float frame) {
		vec3 lp = GetLocalPosition(frame);
		vec3 lr = GetLocalRotationEular(frame);
		vec3 ls = GetLocalScale(frame);

		mat4 translate = translateMaterix(localPosition);
		mat4 rotate = rotateMatrix(localRotationEular);
		mat4 scale = scaleMatrix(localScale);

		return translate * rotate * scale;
	}

	void Object::ResetWorldTransformMatrix() {
		worldTransformMatrix.reset();
	}

	vec3 Object::GetLocalPosition(float frame) {
		if (useAnimation) {
			SKHOLE_UNIMPL();
			return vec3(0);
		}
		else {
			return localPosition;
		}

	}

	vec3 Object::GetLocalRotationEular(float frame) {
		if (useAnimation) {
			SKHOLE_UNIMPL();
			return vec3(0);
		}
		else {
			return localRotationEular;
		}	
	}

	vec3 Object::GetLocalScale(float frame) {
		if (useAnimation) {
			SKHOLE_UNIMPL();
			return vec3(1);
		}
		else {
			return localScale;
		}
	}

	bool Object::IsRoot() {
		return parentObject == nullptr;
	}
	bool Object::IsLear() {
		return childObject == nullptr;
	}
}