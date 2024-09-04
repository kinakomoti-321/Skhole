#pragma once
#include <include.h>
#include <common/log.h>

using namespace VectorLikeGLSL;
namespace Skhole {

	enum class ParameterType
	{
		FLOAT,
		VECTOR,
		COLOR,
		BOOL,
		UINT,
		PARAMETER,
	};

	class Parameter {
	public:
		virtual std::string getParamName() = 0;
		virtual	ParameterType getParamType() = 0;
		virtual void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> value) = 0;
		virtual ShrPtr<Parameter> Copy() = 0;
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

		ShrPtr<Parameter> Copy() override {
			return std::make_shared<ParameterBool>(m_paramName, value);
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

		ShrPtr<Parameter> Copy() override {
			return std::make_shared<ParameterFloat>(m_paramName, value);
		}

		float value;

	private:
		std::string m_paramName;
	};

	class ParameterVector : public Parameter {
	public:
		ParameterVector(std::string name, vec3 v) : m_paramName(name), value(v) {}

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

		ShrPtr<Parameter> Copy() override {
			return std::make_shared<ParameterVector>(m_paramName, value);
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

		ShrPtr<Parameter> Copy() override {
			return std::make_shared<ParameterColor>(m_paramName, value);
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

		ShrPtr<Parameter> Copy() override {
			return std::make_shared<ParameterUint>(m_paramName, value);
		}

		uint32_t value;

	private:
		std::string m_paramName;
	};

	class ParameterParameter : public Parameter {
	public:
		ParameterParameter(std::string name, std::vector<ShrPtr<Parameter>>& v) : m_paramName(name), value(v) {}

		std::string getParamName() override {
			return m_paramName;
		}

		ParameterType getParamType() override {
			return ParameterType::PARAMETER;
		}

		void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> in_value) override {
			SKHOLE_ERROR("Parameter Parameter is not allowed to set value");
		}

		ShrPtr<Parameter> Copy() override {
			return std::make_shared<ParameterParameter>(m_paramName, value);
		}

		std::vector<ShrPtr<Parameter>> value;

	private:
		std::string m_paramName;

	};


	using ParamBool = ParameterBool;
	using ParamFloat = ParameterFloat;
	using ParamVec = ParameterVector;
	using ParamCol = ParameterColor;
	using ParamUint = ParameterUint;
}
