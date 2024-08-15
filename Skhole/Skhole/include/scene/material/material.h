#pragma once
#include <include.h>
#include <scene/material/texture.h>
#include <common/log.h>

using namespace VectorLikeGLSL;

namespace Skhole {
	enum MaterialParameterType
	{
		FLOAT,
		VECTOR,
		COLOR,
		BOOL,
		TEXTURE_ID,
	};


	class MaterialParameter {
	public:
		virtual std::string getParamName() = 0;
		virtual	MaterialParameterType getParamType() = 0;
		virtual void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> value)= 0;
	};

	class MaterialParameterBool : public MaterialParameter{
	public:
		MaterialParameterBool(std::string name, bool v) : m_paramName(name), value(v){}

		std::string getParamName() override {
			return m_paramName;
		}

		MaterialParameterType getParamType() override {
			return MaterialParameterType::BOOL;
		}

		void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> in_value) override {
			if(std::holds_alternative<bool>(in_value))
				value = std::get<bool>(in_value);
			else
				SKHOLE_ERROR("Input Value is different from Material Parameter Type [Bool]");
		}

		bool value;

	private:
		std::string m_paramName;
	};

	class MaterialParameterFloat : public MaterialParameter{
	public:
		MaterialParameterFloat(std::string name, float v) : m_paramName(name), value(v){}

		std::string getParamName() override {
			return m_paramName;
		}

		MaterialParameterType getParamType() override {
			return MaterialParameterType::FLOAT;
		}

		void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> in_value) override {
			if(std::holds_alternative<float>(in_value))
				value = std::get<float>(in_value);
			else
				SKHOLE_ERROR("Input Value is different from Material Parameter Type [Float]");
		}

		float value;

	private:
		std::string m_paramName;
	};

	class MaterialParameterVector : public MaterialParameter{
	public:
		MaterialParameterVector(std::string name, vec3 v) : m_paramName(name), value(v){}

		std::string getParamName() override {
			return m_paramName;
		}

		MaterialParameterType getParamType() override {
			return MaterialParameterType::VECTOR;
		}

		void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> in_value) override {
			if(std::holds_alternative<vec3>(in_value) )
				value = std::get<vec3>(in_value);
			else
				SKHOLE_ERROR("Input Value is different from Material Parameter Type [VECTOR]");
		}

		vec3 value;

	private:
		std::string m_paramName;
	};

	class MaterialParameterColor : public MaterialParameter{
	public:
		MaterialParameterColor(std::string name, vec4 v) : m_paramName(name), value(v){}

		std::string getParamName() override {
			return m_paramName;
		}

		MaterialParameterType getParamType() override {
			return MaterialParameterType::COLOR;
		}

		void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> in_value) override {
			if(std::holds_alternative<vec4>(in_value) )
				value = std::get<vec4>(in_value);
			else
				SKHOLE_ERROR("Input Value is different from Material Parameter Type [COLOR]");
		}

		vec4 value;

	private:
		std::string m_paramName;
	};

	class MaterialParameterTextureID : public MaterialParameter{
	public:
		MaterialParameterTextureID(std::string name, uint32_t v) : m_paramName(name), value(v){}

		std::string getParamName() override {
			return m_paramName;
		}

		MaterialParameterType getParamType() override {
			return MaterialParameterType::TEXTURE_ID;
		}

		void setParamValue(std::variant<float, vec3, vec4, bool, uint32_t> in_value) override {
			if(std::holds_alternative<uint32_t>(in_value))
				value = std::get<uint32_t>(in_value);
			else
				SKHOLE_ERROR("Input Value is different from Material Parameter Type [uint32_t]");
		}

		uint32_t value;

	private:
		std::string m_paramName;
	};

	struct RendererDefinisionMaterial {
		std::vector<ShrPtr<MaterialParameter>> materialParameters;
	};

	using MatParamBool = MaterialParameterBool;
	using MatParamFloat = MaterialParameterFloat;
	using MatParamVector = MaterialParameterVector;
	using MatParamColor = MaterialParameterColor;
	using MatParamTexID = MaterialParameterTextureID;

	struct BasicMaterial {
		vec4 basecolor;
		Texture BaseColorMap;

		float metallic;
		Texture MetallicMap;

		float roughness;
		Texture RoughnessMap;

		Texture NormalMap;
		Texture HeightMap;

		vec3 emissionColor;
		float emissionIntensity;
		Texture EmissiveMap;

		float ior;
		float transmission;
	};

};
