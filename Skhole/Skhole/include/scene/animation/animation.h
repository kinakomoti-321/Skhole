#pragma once
#include <include.h>
#include <math.h>

namespace Skhole {

	inline vec3 LerpFrame(const vec3& v1, const vec3& v2, float f) {
		f = std::clamp(f, 0.0f, 1.0f);
		return v1 * (1.0f - f) + v2 * f;
	}

	inline Quaternion LerpFrame(const Quaternion& q1, const Quaternion& q2, float f) {
		f = std::clamp(f, 0.0f, 1.0f);
		return Slerp(q1, q2, f);
	}

	template <typename T>
	class KeyFrame {
	public:
		KeyFrame() {};
		KeyFrame(T value, int frame) :value(value), frame(frame) {};
		~KeyFrame() {};

		T value;
		float frame;
	};

	template <typename T>
	class Animation {
	public:
		Animation() {};
		~Animation() {};

		void AppendKey(KeyFrame<T> key) {
			keyFrames.push_back(key);
		}

		bool HaveKey() {
			return keyFrames.size() > 0;
		}

		T GetValue(float time)
		{
			if (keyFrames.size() == 0) return T();
			if (keyFrames.size() == 1) return keyFrames[0].value;
			if (time < keyFrames[0].frame) return keyFrames[0].value;
			if (time > keyFrames[keyFrames.size() - 1].frame) return keyFrames[keyFrames.size() - 1].value;

			// Binary Search
			int start = 0;
			int end = keyFrames.size() - 1;
			int mid = (start + end) / 2;

			while (end - start > 1) {
				if (keyFrames[mid].frame < time) {
					start = mid;
				}
				else if (keyFrames[mid].frame > time) {
					end = mid;
				}
				else {
					start = mid;
					end = mid;	
				}

				mid = (start + end) / 2;
			}

			int preIndex = start;
			int nextIndex = start + 1;

			T& preValue = keyFrames[preIndex].value;
			T& nextValue = keyFrames[nextIndex].value;

			float f = (float)(time - keyFrames[preIndex].frame) / (float)(keyFrames[nextIndex].frame - keyFrames[preIndex].frame);

			T value = LerpFrame(preValue, nextValue, f);
			return value;
		}

		std::vector<KeyFrame<T>> keyFrames;
	};


}
