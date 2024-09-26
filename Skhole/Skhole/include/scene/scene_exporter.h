#pragma once

#include <include.h>
#include <scene/scene.h>
#include <scene/animation/animation.h>
#include <nlohmann/json.hpp>

namespace Skhole
{

	inline bool ExportObjects(const std::string& filepath, const std::string& filename, const std::vector<ShrPtr<Object>>& objects)
	{
		std::ofstream file(filepath + filename);
		file << "Object" << std::endl;
		file << objects.size() << std::endl;
		file << std::endl;

		for (auto& obj : objects)
		{
			file << "#Object" << std::endl;
			file << "Name " << obj->objectName << std::endl;
			file << "Type " << ObjectType2Name(obj->GetObjectType()) << std::endl;
			file << "LocalTranslation " << obj->localTranslation.x << " " << obj->localTranslation.y << " " << obj->localTranslation.z << std::endl;
			file << "LocalRotation " << obj->localQuaternion.x << " " << obj->localQuaternion.y << " " << obj->localQuaternion.z << " " << obj->localQuaternion.w << std::endl;
			file << "LocalScale " << obj->localScale.x << " " << obj->localScale.y << " " << obj->localScale.z << std::endl;

			file << "#Animation " << obj->useAnimation << std::endl;
			if (obj->useAnimation)
			{
				auto& animT = obj->translationAnimation;
				auto& animR = obj->rotationAnimation;
				auto& animS = obj->scaleAnimation;

				file << "Translation " << animT.keyFrames.size() << std::endl;
				for (auto& key : animT.keyFrames)
				{
					file << key.frame << " " << key.value.x << " " << key.value.y << " " << key.value.z << std::endl;
				}

				file << "Rotation " << animR.keyFrames.size() << std::endl;
				for (auto& key : animR.keyFrames)
				{
					file << key.frame << " " << key.value.x << " " << key.value.y << " " << key.value.z << " " << key.value.w << std::endl;
				}

				file << "Scale " << animS.keyFrames.size() << std::endl;
				for (auto& key : animS.keyFrames)
				{
					file << key.frame << " " << key.value.x << " " << key.value.y << " " << key.value.z << std::endl;
				}
			}

			file << "#Features" << std::endl;
			if (obj->GetObjectType() == ObjectType::INSTANCE) {
				auto instance = std::dynamic_pointer_cast<Instance>(obj);
				file << "GeometryIndex " << instance->geometryIndex.value() << std::endl;
			}
			else if (obj->GetObjectType() == ObjectType::CAMERA) {
				auto camera = std::dynamic_pointer_cast<CameraObject>(obj);
				file << "Fov " << camera->yFov << std::endl;
			}
			else {
				SKHOLE_UNIMPL();
			}

			file << std::endl;
		}

		file.close();
		return true;
	}

	inline bool ExportGeometries(const std::string& filepath, const std::string& filename, const std::vector<ShrPtr<Geometry>>& geoms) {
		std::ofstream file(filepath + filename);
		file << "Geometries" << std::endl;
		file << geoms.size() << std::endl;
		file << std::endl;

		int index = 0;
		for (const auto& geometry : geoms) {
			file << "#Geom" << index << std::endl;
			file << "#Vertices " << geometry->m_vertices.size() << std::endl;
			for (const auto& vert : geometry->m_vertices) {
				file << "vp " << vert.position.x << " " << vert.position.y << " " << vert.position.z << " " << vert.position.w << std::endl;
				file << "vn " << vert.normal.x << " " << vert.normal.y << " " << vert.normal.z << " " << vert.normal.w << std::endl;
				file << "vt0 " << vert.texcoord0[0] << " " << vert.texcoord0[1] << std::endl;
				file << "vt1 " << vert.texcoord1[0] << " " << vert.texcoord1[1] << std::endl;
				file << "vc " << vert.color.x << " " << vert.color.y << " " << vert.color.z << " " << vert.color.w << std::endl;
			}

			file << "#Indices " << geometry->m_indices.size() << std::endl;
			for (const auto& index : geometry->m_indices) {
				file << index << " ";
			}
			file << std::endl;

			file << "#MaterialIndices " << geometry->m_materialIndices.size() << std::endl;
			for (const auto& index : geometry->m_materialIndices) {
				file << index << " ";
			}
			file << std::endl;

			file << std::endl;

			index++;
		}

		file.close();

		return true;
	}

	inline std::initializer_list<float> toInitializerList(const vec3& v) {
		return { v.x, v.y, v.z };
	}

	inline std::initializer_list<float> toInitializerList(const vec4& v) {
		return { v.x, v.y, v.z ,v.w };
	}

	inline std::string ParameterTypeToName(ParameterType type) {
		switch (type)
		{
		case ParameterType::FLOAT:
			return "FLOAT";
		case ParameterType::UINT:
			return "UINT";
		case ParameterType::BOOL:
			return "BOOL";
		case ParameterType::VECTOR:
			return "VECTOR";
		case ParameterType::COLOR:
			return "COLOR";
		default:
			SKHOLE_UNIMPL();
			break;
		}
	}

	inline ParameterType ParameterNameToType(const std::string& name) {
		if (name == "FLOAT") {
			return ParameterType::FLOAT;
		}
		else if (name == "UINT") {
			return ParameterType::UINT;
		}
		else if (name == "BOOL") {
			return ParameterType::BOOL;
		}
		else if (name == "VECTOR") {
			return ParameterType::VECTOR;
		}
		else if (name == "COLOR") {
			return ParameterType::COLOR;
		}
		else {
			SKHOLE_UNIMPL();
		}
	}

	inline nlohmann::json ParameterToJson(const ShrPtr<Parameter>& param) {
		nlohmann::json pJs;
		ParameterType type = param->getParamType();
		pJs["Name"] = param->getParamName();
		pJs["Type"] = ParameterTypeToName(param->getParamType());

		switch (type)
		{
		case ParameterType::FLOAT:
			pJs["Value"] = GetParamFloatValue(param);
			break;
		case ParameterType::UINT:
			pJs["Value"] = GetParamUintValue(param);
			break;
		case ParameterType::BOOL:
			pJs["Value"] = GetParamBoolValue(param);
			break;
		case ParameterType::VECTOR:
			pJs["Value"] = toInitializerList(GetParamVecValue(param));
			break;
		case ParameterType::COLOR:
			pJs["Value"] = toInitializerList(GetParamColValue(param));
			break;
		default:
			SKHOLE_UNIMPL();
			break;
		}

		return pJs;
	}

	inline bool ExportMaterials(const std::vector<ShrPtr<BasicMaterial>>& bMat, const std::vector<ShrPtr<RendererDefinisionMaterial>>& rMat, nlohmann::json& matJs)
	{
		matJs["NumBasicMaterials"] = bMat.size();
		matJs["NumRendererMaterials"] = rMat.size();

		// Basic Material
		{
			nlohmann::json bMatJs;
			for (int i = 0; i < bMat.size(); i++) {
				auto& mat = bMat[i];
				float baseColor[3] = { mat->basecolor.x, mat->basecolor.y, mat->basecolor.z };
				bMatJs[i] =
				{
					{"Name", mat->materialName},
					{"BaseColor", toInitializerList(mat->basecolor)},
					{"BaseColorTexture", mat->BaseColorMap},

					{"Roughness", mat->roughness},
					{"RoughnessTexture" , mat->RoughnessMap},

					{"Metallic", mat->metallic},
					{"MetallicTexture", mat->MetallicMap},

					{"Sheen", mat->sheen},
					{"Clearcoat", mat->clearcoat},

					{"NormalTexture", mat->NormalMap},
					{"HeightTexture", mat->HeightMap},

					{"EmissionIntensity", mat->emissionIntensity},
					{"EmissionColor", toInitializerList(mat->emissionColor)},
					{"EmissiveTexture", mat->EmissiveMap},

					{"IOR", mat->ior},
					{"Transmission", mat->transmission}
				};
			}

			matJs["BasicMaterials"] = bMatJs;
		}

		// Renderer Material
		{
			nlohmann::json rSumMatJs;
			for (int i = 0; i < rMat.size(); i++) {
				auto& mat = rMat[i];
				nlohmann::json rMatJs;
				rMatJs["Name"] = mat->materialName;
				rMatJs["numParameter"] = mat->materialParameters.size();

				for (int j = 0; j < mat->materialParameters.size(); j++) {
					rMatJs["Parameter"][j] = ParameterToJson(mat->materialParameters[j]);
				}

				rSumMatJs[i] = rMatJs;
			}
			matJs["RendererMaterials"] = rSumMatJs;
		}

		return true;
	}

	inline bool ExportTextures(const std::string& filepath, const std::vector<ShrPtr<Texture>>& texs, nlohmann::json& tJs) {
		std::string texPath = filepath + "/Textures";

		for (int i = 0; i < texs.size(); i++) {
			auto& tex = texs[i];
			std::string texFilename = texPath + "/" + tex->name;
			tex->Export(texFilename);

			tJs[i] = {
				{"Name", tex->name},
				{"FilePath", "./Textures"}
			};
		}

		return true;
	}

	inline bool ExportRendererParameter(const ShrPtr<RendererParameter>& param, nlohmann::json& rParamJs) {
		rParamJs["RendererName"] = param->rendererName;
		rParamJs["sppPerFrame"] = param->sppPerFrame;
		rParamJs["frame"] = param->frame;
		rParamJs["numSPP"] = param->numSPP;
		rParamJs["maxSPP"] = param->maxSPP;

		nlohmann::json rParamExtJs;
		for (int i = 0; i < param->rendererParameters.size(); i++) {
			rParamExtJs["Parameter"][i] = ParameterToJson(param->rendererParameters[i]);
		}

		rParamJs["NumExtensionParameters"] = param->rendererParameters.size();
		rParamJs["ExtensionParameters"] = rParamExtJs;

		nlohmann::json posproJs;
		auto& posproParam = param->posproParameters;
		posproJs["PosproName"] = posproParam.name;
		posproJs["NumParameters"] = posproParam.param.size();
		for (int i = 0; i < posproParam.param.size(); i++) {
			posproJs["Parameter"][i] = ParameterToJson(posproParam.param[i]);
		}

		rParamJs["PostProcess"] = posproJs;

		return true;
	}

	inline bool ExportScene(const std::string& filepath, const std::string& filename, const ShrPtr<Scene>& scene, nlohmann::json& sJs) {
		auto& objects = scene->m_objects;
		auto& geometries = scene->m_geometies;
		auto& materials = scene->m_materials;
		auto& basicMaterials = scene->m_basicMaterials;
		auto& textures = scene->m_textures;
		auto& camera = scene->m_camera;
		auto& rendererParameter = scene->m_rendererParameter;

		sJs["SceneName"] = scene->m_scenenName;

		// Renderer Parameter
		nlohmann::json rParamJs;
		ExportRendererParameter(rendererParameter, rParamJs);
		sJs["RendererParameter"] = rParamJs;

		// Objects
		std::string objectFilename = filename + ".skobj";
		sJs["Objects"] = {
			{"Filename", objectFilename}
		};
		ExportObjects(filepath, objectFilename, objects);

		//Geometry
		std::string geometryFilename = filename + ".skgeom";
		sJs["Geometries"] = {
			{"Filename", geometryFilename}
		};
		ExportGeometries(filepath, geometryFilename, geometries);

		//Materials
		nlohmann::json matJs;
		ExportMaterials(basicMaterials, materials, matJs);
		sJs["Materials"] = matJs;

		//Textures
		nlohmann::json texJs;
		ExportTextures(filepath, textures, texJs);
		sJs["Textures"] = texJs;

		//Camera
		{
			nlohmann::json camJs;
			camJs["Name"] = camera->cameraName;
			camJs["CameraOjbect"]["Use"] = camera->camera != nullptr;
			camJs["CameraOjbect"]["Index"] = scene->m_cameraObjectIndices;

			camJs["Position"] = toInitializerList(camera->position);
			camJs["LookAt"] = toInitializerList(camera->foward);
			camJs["Up"] = toInitializerList(camera->up);
			camJs["Fov"] = camera->fov;

			camJs["Parameters"]["NumParameter"] = camera->extensionParameters.size();
			for (int i = 0; i < camera->extensionParameters.size(); i++) {
				camJs["Parameter"][i] = ParameterToJson(camera->extensionParameters[i]);
			}

			sJs["Camera"] = camJs;
		}

		return true;
	}

	struct ExportSettingDesc {
		std::string filepath;
		std::string filename;
		ShrPtr<Scene> scene;
		OfflineRenderingInfo offlineRenderingInfo;
	};

	inline bool ExportSetting(const ExportSettingDesc& desc) {
		SKHOLE_LOG_SECTION("Save");
		const auto& filepath = desc.filepath;
		const auto& filename = desc.filename;
		auto& offlineInfo = desc.offlineRenderingInfo;

		// Rendering Setting
		nlohmann::json rJs;

		rJs["OfflineRenderingInfo"] = {
			{"Width", offlineInfo.width},
			{"Height", offlineInfo.height},
			{"SPP", offlineInfo.spp},
			{"StartFrame",offlineInfo.startFrame},
			{"EndFrame",offlineInfo.endFrame},
			{"fps",offlineInfo.fps},
			{"Use LimitTime", offlineInfo.useLimitTime },
			{"Limit Time", offlineInfo.limitTime},
			{"Filename", offlineInfo.filename},
			{"Filepath", offlineInfo.filepath}
		};


		nlohmann::json sceneJs;
		// Scene Setting
		ExportScene(filepath, filename, desc.scene, sceneJs);

		rJs["Scene"] = sceneJs;

		std::ofstream write_file(filepath + filename + ".json");
		write_file << rJs.dump(2) << std::endl;
		write_file.close();
		SKHOLE_LOG("Save File : " + filepath + filename + ".json");
		SKHOLE_LOG_SECTION("End Save");

		return true;
	}

}



