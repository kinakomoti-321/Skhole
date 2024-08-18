#pragma once

#include <include.h>

using namespace VectorLikeGLSL;

namespace Skhole {
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


	inline mat4 translateMaterix(const vec3& t) {
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

	inline mat4 rotateMatrix(const vec3& r) {
		mat3 x = rotateX(r.x);
		mat3 y = rotateY(r.y);
		mat3 z = rotateZ(r.z);
		mat3 rot = x * y * z;
		mat4 rotation(0);
		rotation[0] = vec4(rot[0], 0.0f);
		rotation[1] = vec4(rot[1], 0.0f);
		rotation[2] = vec4(rot[2], 0.0f);
		rotation[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

		return rotation;
	}


	inline mat4 scaleMatrix(const vec3& s) {
		mat4 materix(0);
		materix[0][0] = s.x;
		materix[1][1] = s.y;
		materix[2][2] = s.z;
		materix[3][3] = 1.0f;
		return materix;
	}

	struct Quatanion {
		Quatanion() {
			q = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		Quatanion(float x, float y, float z, float w) {
			q = vec4(x, y, z, w);
		}

		Quatanion(const vec4 q) {
			this->q = q;
		}

		Quatanion invert() {
			return Quatanion(-q.x, -q.y, -q.z, q.w);
		}

		vec4 q;
	};

	inline Quatanion operator+(const Quatanion& a, const Quatanion& b) {
		return Quatanion(a.q + b.q);
	}

	inline Quatanion operator-(const Quatanion& a, const Quatanion& b) {
		return Quatanion(a.q - b.q);
	}

	inline Quatanion operator*(const Quatanion& a, const Quatanion& b) {
		return Quatanion(
			a.q.w * b.q.x + a.q.x * b.q.w + a.q.y * b.q.z - a.q.z * b.q.y,
			a.q.w * b.q.y + a.q.y * b.q.w + a.q.z * b.q.x - a.q.x * b.q.z,
			a.q.w * b.q.z + a.q.z * b.q.w + a.q.x * b.q.y - a.q.y * b.q.x,
			a.q.w * b.q.w - a.q.x * b.q.x - a.q.y * b.q.y - a.q.z * b.q.z
		);
	}

	inline Quatanion QuatanionFromAxis(float theta, vec3 axis) {
		float s = std::sin(theta / 2.0f);
		float c = std::cos(theta / 2.0f);
		return Quatanion(axis.x * s, axis.y * s, axis.z * s, c);
	}

	inline mat4 rotateMatrixFromQuatanion(const Quatanion& q) {
		float x = q.q.x;
		float y = q.q.y;
		float z = q.q.z;
		float w = q.q.w;

		mat4 rotation(0);
		rotation[0][0] = 1.0f - 2.0f * y * y - 2.0f * z * z;
		rotation[0][1] = 2.0f * x * y - 2.0f * z * w;
		rotation[0][2] = 2.0f * x * z + 2.0f * y * w;

		rotation[1][0] = 2.0f * x * y + 2.0f * z * w;
		rotation[1][1] = 1.0f - 2.0f * x * x - 2.0f * z * z;
		rotation[1][2] = 2.0f * y * z - 2.0f * x * w;

		rotation[2][0] = 2.0f * x * z - 2.0f * y * w;
		rotation[2][1] = 2.0f * y * z + 2.0f * x * w;
		rotation[2][2] = 1.0f - 2.0f * x * x - 2.0f * y * y;

		rotation[3][3] = 1.0f;

		return rotation;
	}

	inline mat4 rotateMaterixFromAxis(float theta,const vec3& n) {
		mat4 rotation(0);
		float s = std::sin(theta);
		float c = std::cos(theta);

		rotation[0][0] = c + (1 - c) * n.x * n.x;
		rotation[0][1] = n.x * n.y * (1 - c) - n.z * s;
		rotation[0][2] = n.z * n.x * (1 - c) + n.y * s;

		rotation[1][0] = n.x * n.y * (1 - c) + n.z * s;
		rotation[1][1] = c + (1 - c) * n.y * n.y;
		rotation[1][2] = n.y * n.z * (1 - c) - n.x * s;

		rotation[2][0] = n.z * n.x * (1 - c) - n.y * s;
		rotation[2][1] = n.y * n.z * (1 - c) + n.x * s;
		rotation[2][2] = c + (1 - c) * n.z * n.z;

		rotation[3][3] = 1.0f;
		
		return rotation;
	}
}
