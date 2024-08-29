#pragma once

#include <include.h>
#include <scene/object/object.h>
#include <scene/object/geometry.h>
#include <scene/material/texture.h>
#include <scene/material/material.h>
#include <scene/object/instance.h>
namespace Skhole {

	// For GLTF Binary Data
	template <typename T>
	struct ArrayAdapter {
		const unsigned char* dataPtr;
		const size_t count;
		const size_t byteStrinde;

		ArrayAdapter(const unsigned char* dataPtr, const size_t count, const size_t byteStrinde)
			: dataPtr(dataPtr), count(count), byteStrinde(byteStrinde) {}

		T operator[](size_t index) const {
			if (index >= count) SKHOLE_ERROR("Index out of range");
			return *(reinterpret_cast<const T*>(dataPtr + index * byteStrinde));
		}
	};

	struct IntArrayBase {
		virtual ~IntArrayBase() = default;
		virtual uint32_t operator[](size_t index) const = 0;
		virtual size_t size() const = 0;
	};

	template <typename T>
	struct IntArray {
		UnqPtr<ArrayAdapter<T>> adapter;
		IntArray(const unsigned char* dataPtr, const size_t count, const size_t byteStride)
			: adapter(MakeUnq<ArrayAdapter<T>>(dataPtr, count, byteStride)) {}

		uint32_t operator[](size_t index) const {
			return static_cast<uint32_t>((*adapter)[index]);
		}
	};

	struct RealArrayBase {
		virtual ~RealArrayBase() = default;
		virtual float operator[](size_t index) const = 0;
		virtual size_t size() const = 0;
	};

	template <typename T>
	struct RealArray {
		UnqPtr<ArrayAdapter<T>> adapter;
		RealArray(const unsigned char* dataPtr, const size_t count, const size_t byteStride)
			: adapter(MakeUnq<ArrayAdapter<T>>(dataPtr, count, byteStride)) {}

		T operator[](size_t index) const {
			return static_cast<T>((*adapter)[index]);
		}
	};

	template <typename T>
	struct v2 {
		T x, y;
	};

	template <typename T>
	struct v3 {
		T x, y, z;
	};

	template <typename T>
	struct v4 {
		T x, y, z, w;
	};

	using v2f = v2<float>;
	using v3f = v3<float>;
	using v4f = v4<float>;
	using v2d = v2<double>;
	using v3d = v3<double>;
	using v4d = v4<double>;

	struct v2fArray {
		UnqPtr<ArrayAdapter<v2f>> adapter;
		v2fArray(const unsigned char* dataPtr, const size_t count, const size_t byteStride) {
			adapter = MakeUnq<ArrayAdapter<v2f>>(dataPtr, count, byteStride);
		}
		v2f operator[](size_t index) const {
			return (*adapter)[index];
		}
	};

	struct v3fArray {
		UnqPtr<ArrayAdapter<v3f>> adapter;
		v3fArray(const unsigned char* dataPtr, const size_t count, const size_t byteStride) {
			adapter = MakeUnq<ArrayAdapter<v3f>>(dataPtr, count, byteStride);
		}
		v3f operator[](size_t index) const {
			return (*adapter)[index];
		}
	};

	struct v4fArray {
		UnqPtr<ArrayAdapter<v4f>> adapter;
		v4fArray(const unsigned char* dataPtr, const size_t count, const size_t byteStride) {
			adapter = MakeUnq<ArrayAdapter<v4f>>(dataPtr, count, byteStride);
		}
		v4f operator[](size_t index) const {
			return (*adapter)[index];
		}
	};

	struct v2dArray {
		UnqPtr<ArrayAdapter<v2d>> adapter;
		v2dArray(const unsigned char* dataPtr, const size_t count, const size_t byteStride) {
			adapter = MakeUnq<ArrayAdapter<v2d>>(dataPtr, count, byteStride);
		}
		v2d operator[](size_t index) const {
			return (*adapter)[index];
		}
	};

	struct v3dArray {
		UnqPtr<ArrayAdapter<v3d>> adapter;
		v3dArray(const unsigned char* dataPtr, const size_t count, const size_t byteStride) {
			adapter = MakeUnq<ArrayAdapter<v3d>>(dataPtr, count, byteStride);
		}
		v3d operator[](size_t index) const {
			return (*adapter)[index];
		}
	};

	struct v4dArray {
		UnqPtr<ArrayAdapter<v4d>> adapter;
		v4dArray(const unsigned char* dataPtr, const size_t count, const size_t byteStride) {
			adapter = MakeUnq<ArrayAdapter<v4d>>(dataPtr, count, byteStride);
		}
		v4d operator[](size_t index) const {
			return (*adapter)[index];
		}
	};

	bool LoadGLTFFile(
		const std::string& filename,
		std::vector<ShrPtr<Object>>& inObjects,
		std::vector<ShrPtr<Geometry>>& inGeometies,
		std::vector<ShrPtr<BasicMaterial>>& inBasicMaterials,
		std::vector<ShrPtr<Texture>>& inTextures
	);
}
