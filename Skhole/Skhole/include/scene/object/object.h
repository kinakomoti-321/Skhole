#pragma once

#include <include.h>

namespace Skhole {

	enum ObjectType {
		TypeInstance,
		TypeLight,
		TypeVolume
	};

	class Object {
	
	public:
		Object(){};
		~Object(){};

		virtual std::string GetObjectName() = 0; 
		virtual ObjectType GetObjectType() = 0;
	};
};
