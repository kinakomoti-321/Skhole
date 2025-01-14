#include <scene/object/object.h>

using namespace VectorLikeGLSL;
namespace Skhole {
	mat4 Object::SetWorldTransformMatrix(float time) {
		localQuaternion = Normalize(localQuaternion);

		if (worldTransformMatrix.has_value()) {
			return worldTransformMatrix.value();
		}

		if (IsRoot()) {
			worldTransformMatrix = GetTransformMatrix(time);
		}
		else {
			worldTransformMatrix = GetTransformMatrix(time) * parentObject->SetWorldTransformMatrix(time) ;
		}

		return worldTransformMatrix.value();
	}

	mat4 Object::GetWorldTransformMatrix(float time) {
		if (worldTransformMatrix.has_value())
		{
			return worldTransformMatrix.value();
		}
		else
		{
			return SetWorldTransformMatrix(time);
		}
	}

	mat4 Object::GetTransformMatrix(float time) {
		vec3 lp = GetTranslation(time);
		Quaternion lr = GetRotation(time);
		vec3 ls = GetScale(time);

		mat4 translate = TranslateAffine(lp);
		mat4 rotate = RotateAffine(lr);
		mat4 scale = ScaleAffine(ls);

		return scale * rotate * translate;
	}

	void Object::ResetWorldTransformMatrix() {
		worldTransformMatrix.reset();
		if (!IsLear()) childObject->ResetWorldTransformMatrix();
	}

	vec3 Object::GetTranslation(float time) {
		if (useAnimation)
		{
			return translationAnimation.GetValue(time);
		}
		else {
			return localTranslation;
		}

	}

	Quaternion Object::GetRotation(float time) {
		if (useAnimation)
		{
			return rotationAnimation.GetValue(time);
		}
		else
		{
			return localQuaternion;
		}

	}

	vec3 Object::GetScale(float time) {
		if (useAnimation)
		{
			return scaleAnimation.GetValue(time);
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