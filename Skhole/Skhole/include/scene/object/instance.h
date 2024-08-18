#pragma once
#include <include.h>
#include <scene/object/object.h>
#include <scene/animation/animation.h>
#include <common/math.h>

using namespace VectorLikeGLSL;
namespace Skhole {

	class Instance :public Object{
	public:
		Instance(){};
		~Instance(){};

		ObjectType GetObjectType() override {return ObjectType::INSTANCE;};

	public:
		std::optional<uint32_t> geometryIndex;
	};

}
