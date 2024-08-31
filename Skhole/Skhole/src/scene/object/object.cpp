#include <scene/object/object.h>

using namespace VectorLikeGLSL;
namespace Skhole {
	mat4 Object::SetWorldTransformMatrix(float frame) {
		localQuaternion = Normalize(localQuaternion);

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
		Quaternion lr = GetLocalRotation(frame);
		vec3 ls = GetLocalScale(frame);

		mat4 translate = TranslateAffine(lp);
		mat4 rotate = RotateAffine(lr);
		mat4 scale = ScaleAffine(ls);

		return scale * rotate *  translate;
	}

	void Object::ResetWorldTransformMatrix() {
		worldTransformMatrix.reset();
		if (!IsLear()) childObject->ResetWorldTransformMatrix();
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

	Quaternion Object::GetLocalRotation(float frame) {
		if (useAnimation) {
			SKHOLE_UNIMPL();
			return Quaternion();
		}
		else {
			return localQuaternion;
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