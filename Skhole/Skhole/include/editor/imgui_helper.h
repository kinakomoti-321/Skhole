#pragma once
#include <include.h>
#include <scene/parameter/parameter.h>

namespace Skhole {

	inline bool InputUint(const char* name, unsigned int* value) {
		return ImGui::InputScalar(name, ImGuiDataType_U32, value);
	}

	inline void StringText(const std::string str) {
		ImGui::Text(str.c_str());
	}

	inline bool ParameterGUI(std::vector<ShrPtr<Parameter>>& parameter) {
		bool active = false;
		for (auto& param : parameter) {
			if (param->getParamType() == ParameterType::FLOAT) {
				auto floatParam = std::static_pointer_cast<ParameterFloat>(param);
				active |= ImGui::SliderFloat(param->getParamName().c_str(), &floatParam->value, floatParam->minValue, floatParam->maxValue);
			}
			else if (param->getParamType() == ParameterType::BOOL) {
				auto boolParam = std::static_pointer_cast<ParameterBool>(param);
				active |= ImGui::Checkbox(param->getParamName().c_str(), &boolParam->value);
			}
			else if (param->getParamType() == ParameterType::VECTOR) {
				auto vecParam = std::static_pointer_cast<ParameterVector>(param);
				active |= ImGui::InputFloat3(param->getParamName().c_str(), vecParam->value.v);
			}
			else if (param->getParamType() == ParameterType::UINT) {
				auto uintParam = std::static_pointer_cast<ParameterUint>(param);
				active |= InputUint(param->getParamName().c_str(), &uintParam->value);
			}
			else if (param->getParamType() == ParameterType::COLOR) {
				auto colorParam = std::static_pointer_cast<ParameterColor>(param);
				active |= ImGui::ColorEdit3(param->getParamName().c_str(), colorParam->value.v);
			}
			else if (param->getParamType() == ParameterType::PARAMETER) {
				auto paramParam = std::static_pointer_cast<ParameterParameter>(param);
				if (ImGui::TreeNode(param->getParamName().c_str())) {
					active |= ParameterGUI(paramParam->value);
					ImGui::TreePop();
				}
			}
			else {
				SKHOLE_UNIMPL("not implement");
			}
		}

		return active;
	}

}
