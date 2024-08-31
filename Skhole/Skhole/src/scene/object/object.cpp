#include <scene/object/object.h>

using namespace VectorLikeGLSL;
namespace Skhole {
	mat4 Object::SetWorldTransformMatrix(int frame) {
		localQuaternion = Normalize(localQuaternion);

		if (worldTransformMatrix.has_value()) {
			return worldTransformMatrix.value();
		}

		if (IsRoot()) {
			worldTransformMatrix = GetTransformMatrix(frame);
		}
		else {
			worldTransformMatrix = parentObject->SetWorldTransformMatrix(frame) * GetTransformMatrix(frame);
		}

		return worldTransformMatrix.value();
	}

	mat4 Object::GetWorldTransformMatrix(int frame) {
		if (worldTransformMatrix.has_value())
		{
			return worldTransformMatrix.value();
		}
		else
		{
			return SetWorldTransformMatrix(frame);
		}
	}

	mat4 Object::GetTransformMatrix(int frame) {
		vec3 lp = GetTranslation(frame);
		Quaternion lr = GetRotation(frame);
		vec3 ls = GetScale(frame);

		mat4 translate = TranslateAffine(lp);
		mat4 rotate = RotateAffine(lr);
		mat4 scale = ScaleAffine(ls);

		return scale * rotate * translate;
	}

	void Object::ResetWorldTransformMatrix() {
		worldTransformMatrix.reset();
		if (!IsLear()) childObject->ResetWorldTransformMatrix();
	}

	vec3 Object::GetTranslation(int frame) {
		if (useAnimation)
		{
			return translationAnimation.GetValue(frame);
		}
		else {
			return localTranslation;
		}

	}

	Quaternion Object::GetRotation(int frame) {
		if (useAnimation)
		{
			return rotationAnimation.GetValue(frame);
		}
		else
		{
			return localQuaternion;
		}

	}

	vec3 Object::GetScale(int frame) {
		if (useAnimation)
		{
			return scaleAnimation.GetValue(frame);
		}
		else
		{
			return localScale;
		}
	}

	bool Object::IsRoot() {
		return parentObject == nullptr;
	}
	bool Object::IsLear() {
		return childObject == nullptr;
	}

	void Object::SetAnimationKey() {
		useAnimation = translationAnimation.HaveKey() || rotationAnimation.HaveKey() || scaleAnimation.HaveKey();

		if (!translationAnimation.HaveKey()) {
			translationAnimation.AppendKey(KeyFrame<vec3>(vec3(0, 0, 0), 0));
		}
		if (!rotationAnimation.HaveKey()) {
			rotationAnimation.AppendKey(KeyFrame<Quaternion>(Quaternion(0, 0, 0, 1), 0));
		}
		if (!scaleAnimation.HaveKey()) {
			scaleAnimation.AppendKey(KeyFrame<vec3>(vec3(1, 1, 1), 0));
		}
	}
}