#pragma once

#include <include.h>

using namespace VectorLikeGLSL;

namespace Skhole {
	constexpr float PI = 3.14159265358979323846f;
	constexpr float TAU = 6.28318530717958647692f;

	inline mat4 IdentityMat4() {
		mat4 identity(0);
		identity[0][0] = 1.0f;
		identity[1][1] = 1.0f;
		identity[2][2] = 1.0f;
		identity[3][3] = 1.0f;
		return identity;
	}

	inline mat2 rotMat(float theta) {
		float s = std::sin(theta);
		float c = std::cos(theta);
		return mat2(c, -s, s, c);
	}

	inline mat3 rotateX(float theta) {
		float s = std::sin(theta);
		float c = std::cos(theta);
		return mat3(1.0f, 0.0f, 0.0f, 0.0f, c, -s, 0.0f, s, c);
	}

	inline mat3 rotateY(float theta) {
		float s = std::sin(theta);
		float c = std::cos(theta);
		return mat3(c, 0.0f, s, 0.0f, 1.0f, 0.0f, -s, 0.0f, c);
	}

	inline mat3 rotateZ(float theta) {
		float s = std::sin(theta);
		float c = std::cos(theta);
		return mat3(c, -s, 0.0f, s, c, 0.0f, 0.0f, 0.0f, 1.0f);
	}


	inline mat4 TranslateAffine(const vec3& t) {
		mat4 materix(0);
		materix[0][0] = 1.0f;
		materix[1][1] = 1.0f;
		materix[2][2] = 1.0f;
		materix[3][3] = 1.0f;
		materix[0][3] = t.x;
		materix[1][3] = t.y;
		materix[2][3] = t.z;
		return materix;
	}

	//inline mat4 RotateAffine(const vec3& r) {
	//	mat3 x = rotateX(r.x);
	//	mat3 y = rotateY(r.y);
	//	mat3 z = rotateZ(r.z);
	//	mat3 rot = x * y * z;
	//	mat4 rotation(0);
	//	rotation[0] = vec4(rot[0], 0.0f);
	//	rotation[1] = vec4(rot[1], 0.0f);
	//	rotation[2] = vec4(rot[2], 0.0f);
	//	rotation[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	//	return rotation;
	//}


	inline mat4 ScaleAffine(const vec3& s) {
		mat4 materix(0);
		materix[0][0] = s.x;
		materix[1][1] = s.y;
		materix[2][2] = s.z;
		materix[3][3] = 1.0f;
		return materix;
	}

	// Quaternion (vector, scalar)
	struct Quaternion {
		float x, y, z, w;

		Quaternion() : x(0), y(0), z(0), w(1) {}
		Quaternion(float s) : x(s), y(s), z(s), w(s) {}
		Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
		Quaternion(vec3 v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
	};

	inline Quaternion operator+(const Quaternion& a, const Quaternion& b) {
		Quaternion q;
		q.x = a.x + b.x;
		q.y = a.y + b.y;
		q.z = a.z + b.z;
		q.w = a.w + b.w;
		return q;
	}

	inline Quaternion operator-(const Quaternion& a, const Quaternion& b) {
		Quaternion q;
		q.x = a.x - b.x;
		q.y = a.y - b.y;
		q.z = a.z - b.z;
		q.w = a.w - b.w;
		return q;
	}

	inline Quaternion operator*(const Quaternion& a, const Quaternion& b) {
		return Quaternion(
			a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
			a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z,
			a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x,
			a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
		);
	}

	inline Quaternion operator*(const Quaternion& q, float s) {
		return Quaternion(q.x * s, q.y * s, q.z * s, q.w * s);
	}

	inline Quaternion operator*(float s, const Quaternion& q) {
		return Quaternion(q.x * s, q.y * s, q.z * s, q.w * s);
	}

	inline Quaternion operator/(const Quaternion& q, float s) {
		return Quaternion(q.x / s, q.y / s, q.z / s, q.w / s);
	}

	inline Quaternion operator/(float s, const Quaternion& q) {
		return Quaternion(s / q.x, s / q.y, s / q.z, s / q.w);
	}

	inline float Length(const Quaternion& q) {
		return std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	}

	inline float Length2(const Quaternion& q) {
		return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	}

	inline Quaternion Conjugate(const Quaternion& q) {
		return Quaternion(-q.x, -q.y, -q.z, q.w);
	}

	inline Quaternion Inverse(const Quaternion& q) {
		return Quaternion(-q.x, -q.y, -q.z, q.w) / (q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	}

	inline Quaternion Normalize(const Quaternion& q) {
		return q / Length(q);
	}

	inline Quaternion QuatanionFromAxis(float theta, vec3 axis) {
		float s = std::sin(theta / 2.0f);
		float c = std::cos(theta / 2.0f);
		return Quaternion(axis.x * s, axis.y * s, axis.z * s, c);
	}

	inline mat3 RotationMatrix(const Quaternion& q)
	{
		return mat3(
			2 * q.w * q.w + 2 * q.x * q.x - 1, 2 * q.x * q.y - 2 * q.z * q.w, 2 * q.x * q.z + 2 * q.y * q.w,
			2 * q.x * q.y + 2 * q.z * q.w, 2 * q.w * q.w + 2 * q.y * q.y - 1, 2 * q.y * q.z - 2 * q.x * q.w,
			2 * q.x * q.z - 2 * q.y * q.w, 2 * q.y * q.z + 2 * q.x * q.w, 2 * q.w * q.w + 2 * q.z * q.z - 1
		);
	}

	inline mat4 RotateAffine(const Quaternion& q)
	{
		return mat4
		(
			2 * q.w * q.w + 2 * q.x * q.x - 1, 2 * q.x * q.y - 2 * q.z * q.w, 2 * q.x * q.z + 2 * q.y * q.w, 0,
			2 * q.x * q.y + 2 * q.z * q.w, 2 * q.w * q.w + 2 * q.y * q.y - 1, 2 * q.y * q.z - 2 * q.x * q.w, 0,
			2 * q.x * q.z - 2 * q.y * q.w, 2 * q.y * q.z + 2 * q.x * q.w, 2 * q.w * q.w + 2 * q.z * q.z - 1, 0,
			0, 0, 0, 1
		);
	}

	enum RotationOrder {
		XYZ,
		XZY,
		YXZ,
		YZX,
		ZXY,
		ZYX
	};

	inline Quaternion EularToQuaternion(const vec3& eular, RotationOrder order) {
		Quaternion qx = QuatanionFromAxis(eular.x, vec3(1, 0, 0));
		Quaternion qy = QuatanionFromAxis(eular.y, vec3(0, 1, 0));
		Quaternion qz = QuatanionFromAxis(eular.z, vec3(0, 0, 1));

		switch (order) {
		case RotationOrder::XYZ:
			return qx * qy * qz;
			break;
		default:
			SKHOLE_UNIMPL();
			break;
		}
	}

	inline vec3 QuaternionToEular(const Quaternion& q, RotationOrder order) {
		mat3 m = RotationMatrix(q);
		vec3 eular(0);
		switch (order)
		{
		case RotationOrder::XYZ:
			eular.y = std::asin(-m[0][2]);
			if (abs(std::cos(eular.y)) < 0.0001f)
			{
				eular.x = std::atan(-m[1][2] / m[2][2]);
				eular.z = std::atan(-m[0][1] / m[0][0]);
			}
			else
			{
				eular.x = std::atan(m[2][1] / m[2][2]);
				eular.z = 0.0f;
			}
			break;
		default:
			SKHOLE_UNIMPL();
			break;
		}

		return eular;
	}

	inline mat3 Inverse3x3(const mat3& m) {
		float det =
			m[0][0] * m[1][1] * m[2][2]
			+ m[0][1] * m[1][2] * m[2][0]
			+ m[0][2] * m[1][0] * m[2][1]
			- m[0][2] * m[1][1] * m[2][0]
			- m[0][1] * m[1][0] * m[2][2]
			- m[0][0] * m[1][2] * m[2][1];

		return mat3(
			(m[1][1] * m[2][2] - m[1][2] * m[2][1]) / det,
			(m[0][2] * m[2][1] - m[0][1] * m[2][2]) / det,
			(m[0][1] * m[1][2] - m[0][2] * m[1][1]) / det,
			(m[1][2] * m[2][0] - m[1][0] * m[2][2]) / det,
			(m[0][0] * m[2][2] - m[0][2] * m[2][0]) / det,
			(m[0][2] * m[1][0] - m[0][0] * m[1][2]) / det,
			(m[1][0] * m[2][1] - m[1][1] * m[2][0]) / det,
			(m[0][1] * m[2][0] - m[0][0] * m[2][1]) / det,
			(m[0][0] * m[1][1] - m[0][1] * m[1][0]) / det
		);
	}

	inline mat3 Transpose3x3(const mat3& m) {
		return mat3(
			m[0][0], m[1][0], m[2][0],
			m[0][1], m[1][1], m[2][1],
			m[0][2], m[1][2], m[2][2]
		);
	}

	inline mat3 NormalTransformMatrix3x3(const mat4& m) {
		mat3 m3x3(0);
		m3x3[0] = m[0].xyz;
		m3x3[1] = m[1].xyz;
		m3x3[2] = m[2].xyz;

		return Transpose3x3(Inverse3x3(m3x3));
	}

	inline Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, const float f) {
		float dotq = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
		float term = 1 - dotq * dotq;
		if (term <= 0.0001)return q1;
		float sinPhi = std::sqrt(term);

		float phi = std::asin(sinPhi);

		return (q1 * std::sin((1 - f) * phi) + q2 * std::sin(f * phi)) / sinPhi;

	}

	inline float DegreeToRadian(float degree) {
		return degree * TAU / 360.0f;
	}

	inline float RadianToDegree(float radian) {
		return radian * 360.0f / TAU;
	}
}
