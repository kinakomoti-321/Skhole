#pragma once
#include <include.h>
#include <math.h>

namespace Skhole {

	vec3 LerpFrame(const vec3& v1, const vec3& v2, float f) {
		f = std::clamp(f, 0.0f, 1.0f);
		return v1 * (1.0f - f) + v2 * f;
	}

	Quaternion LerpFrame(const Quaternion& q1, const Quaternion& q2, float f) {
		f = std::clamp(f, 0.0f, 1.0f);
		return Slerp(q1, q2, f);
	}

	template <typename T>
	class KeyFrame {
	public:
		KeyFrame() {};
		~KeyFrame() {};

		T value;
		int frame;
	};

	template <typename T>
	class Animation {
	public:
		Animation() {};
		~Animation() {};

		T GetValue(int frame)
		{
			if (keyFrames.size() == 0) return T();
			if (keyFrames.size() == 1) return keyFrames[0].value;
			if (frame < keyFrames[0].frame) return keyFrames[0].value;
			if (frame > keyFrames[keyFrames.size() - 1].frame) return keyFrames[keyFrames.size() - 1].value;

			// Binary Search
			int start = 0;
			int end = keyFrames.size() - 1;
			int mid = (start + end) / 2;


			int preIndex = 0;
			int nextIndex = 0;

			while (true) {
				if (keyFrames[mid].frame = frame) {
					preIndex = mid;
					nextIndex = mid;
					break;
				}
				else if (keyFrames[mid].frame < frame || frame < keyFrames[mid + 1].frame) {
					preIndex = mid;
					nextIndex = mid + 1;
					break;
				}
				else if (keyFrames[mid].frame > frame || frame > keyFrames[mid - 1].frame) {
					preIndex = mid - 1;
					nextIndex = mid;
					break;
				}
				else if (keyFrames[mid].frame > frame) {
					end = mid;
				}
				else if (keyFrames[mid].frame < frame) {
					start = mid;
				}

				mid = (start + end) / 2;
			}


			T& preValue = keyFrames[preIndex].value;
			T& nextValue = keyFrames[nextIndex].value;

			float f = (float)(frame - preIndex) / (float)(nextIndex - preIndex);

			T value = FrameLerp(preValue, nextValue, f);
			return value;
		}

		std::vector<KeyFrame<T>> keyFrames;
	};


}
