#pragma once
#include <include.h>
#include <scene/object/object.h>
#include <scene/animation/animation.h>

using namespace VectorLikeGLSL;
namespace Skhole {

	class Instance :public Object{
	public:
		Instance(){};
		~Instance(){};

		std::string GetObjectName() override{
			return m_instanceName;
		}; 

		ObjectType GetObjectType() override{
			return ObjectType::TypeInstance;
		};

		mat4 GetLocalTransformMatrix(float frame);
		vec3 GetLocalTranslation(float frame) 
		{
			return vec3(0.0);
		};

		vec3 GetLocalRotation(float frame) 
		{
			return vec3(0.0);
		};

		vec3 GetLocalScale(float frame)
		{
			return vec3(0.0);
		};

		bool IsLeaf() const
		{
			return m_childInstanceIndex.has_value();
		}

		bool IsRoot() const
		{
			return !m_parentInstanceIndex.has_value();	
		}

	public:
		std::optional<uint32_t> m_childInstanceIndex;				
		std::optional<uint32_t> m_parentInstanceIndex;

		std::optional<uint32_t> m_geometryIndex;

		std::string m_instanceName;

		//AnimationData m_animationData;
	};

}
