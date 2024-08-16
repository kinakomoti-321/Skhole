#pragma once
#include <include.h>
#include <common/log.h>

using namespace VectorLikeGLSL;
namespace Skhole {

	enum ParameterType
	{
		FLOAT,
		VECTOR,
		COLOR,
		BOOL,
		UINT,
	};


	class Parameter {
	public:
		virtual std::string getParamName() = 0;
		virtual	ParameterType getParamType() = 0;
		virtual void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> value) = 0;
	};

	class ParameterBool : public Parameter {
	public:
		ParameterBool(std::string name, bool v) : m_paramName(name), value(v) {}

		std::string getParamName() override {
			return m_paramName;
		}

		ParameterType getParamType() override {
			return ParameterType::BOOL;
		}

		void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> in_value) override {
			if (std::holds_alternative<bool>(in_value))
				value = std::get<bool>(in_value);
			else
				SKHOLE_ERROR("Input Value is different from Material Parameter Type [Bool]");
		}

		bool value;

	private:
		std::string m_paramName;
	};

	class ParameterFloat : public Parameter {
	public:
		ParameterFloat(std::string name, float v) : m_paramName(name), value(v) {}

		std::string getParamName() override {
			return m_paramName;
		}

		ParameterType getParamType() override {
			return ParameterType::FLOAT;
		}

		void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> in_value) override {
			if (std::holds_alternative<float>(in_value))
				value = std::get<float>(in_value);
			else
				SKHOLE_ERROR("Input Value is different from Material Parameter Type [Float]");
		}

		float value;

	private:
		std::string m_paramName;
	};

	class ParamterVector : public Parameter {
	public:
		ParamterVector(std::string name, vec3 v) : m_paramName(name), value(v) {}

		std::string getParamName() override {
			return m_paramName;
		}

		ParameterType getParamType() override {
			return ParameterType::VECTOR;
		}

		void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> in_value) override {
			if (std::holds_alternative<vec3>(in_value))
				value = std::get<vec3>(in_value);
			else
				SKHOLE_ERROR("Input Value is different from Material Parameter Type [VECTOR]");
		}

		vec3 value;

	private:
		std::string m_paramName;
	};

	class ParameterColor : public Parameter {
	public:
		ParameterColor(std::string name, vec4 v) : m_paramName(name), value(v) {}

		std::string getParamName() override {
			return m_paramName;
		}

		ParameterType getParamType() override {
			return ParameterType::COLOR;
		}

		void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> in_value) override {
			if (std::holds_alternative<vec4>(in_value))
				value = std::get<vec4>(in_value);
			else
				SKHOLE_ERROR("Input Value is different from Material Parameter Type [COLOR]");
		}

		vec4 value;

	private:
		std::string m_paramName;
	};

	class ParameterUint : public Parameter {
	public:
		ParameterUint(std::string name, uint32_t v) : m_paramName(name), value(v) {}

		std::string getParamName() override {
			return m_paramName;
		}

		ParameterType getParamType() override {
			return ParameterType::UINT;
		}

		void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> in_value) override {
			if (std::holds_alternative<uint32_t>(in_value))
				value = std::get<uint32_t>(in_value);
			else
				SKHOLE_ERROR("Input Value is different from Material Parameter Type [uint32_t]");
		}

		uint32_t value;

	private:
		std::string m_paramName;
	};

	using ParamBool = ParameterBool;
	using ParamFloat = ParameterFloat;
	using ParamVec = ParamterVector;
	using ParamCol = ParameterColor;
	using ParamUint = ParameterUint;
}
