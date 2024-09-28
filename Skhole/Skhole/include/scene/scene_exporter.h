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

		for (int i = 0; i < objects.size(); i++) {
			auto& obj = objects[i];
			obj->objectIndex = i;
		}

		for (auto& obj : objects)
		{
			file << "#Object" << std::endl;
			file << "Name " << obj->objectName << std::endl;

			file << "#Features" << std::endl;
			file << "Type " << ObjectType2Name(obj->GetObjectType()) << std::endl;
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

			file << "#Transform" << std::endl;
			file << "LocalTranslation " << obj->localTranslation.x << " " << obj->localTranslation.y << " " << obj->localTranslation.z << std::endl;
			file << "LocalRotation " << obj->localQuaternion.x << " " << obj->localQuaternion.y << " " << obj->localQuaternion.z << " " << obj->localQuaternion.w << std::endl;
			file << "LocalScale " << obj->localScale.x << " " << obj->localScale.y << " " << obj->localScale.z << std::endl;

			file << "#Parent_Child" << std::endl;
			if (obj->haveParent()) {
				file << "Parent " << obj->parentObject->objectIndex << std::endl;
			}
			else {
				file << "Parent -1" << std::endl;
			}
			if (obj->haveChild())
			{
				file << "Child " << obj->childObject->objectIndex << std::endl;
			}
			else {
				file << "Child -1" << std::endl;
			}

			file << "#Animation " << obj->useAnimation << std::endl;
			if (obj->useAnimation)
			{
				auto& animT = obj->translationAnimation;
				bool haveTransform = animT.keyFrames.size() > 0;
				auto& animR = obj->rotationAnimation;
				bool haveRotation = animR.keyFrames.size() > 0;
				auto& animS = obj->scaleAnimation;
				bool haveScale = animS.keyFrames.size() > 0;

				file << "NumAnimation " << (int)haveTransform + (int)haveRotation + (int)haveScale << std::endl;
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

		rParamJs["NumParameters"] = param->rendererParameters.size();
		rParamJs["Parameters"] = rParamExtJs;

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

	struct ImportSettingOutput {
		bool success;
		ShrPtr<Scene> scene;
		OfflineRenderingInfo offlineRenderingInfo;
	};

	inline bool ImportObjects(const std::string& path, std::vector<ShrPtr<Object>>& objects) {
		std::ifstream read_file(path);

		if (!read_file.is_open()) {
			SKHOLE_LOG("Failed to open file : " + path);
			return false;
		}

		std::string prefix;
		read_file >> prefix;

		if (prefix != "Object") return false;

		int numObj;
		read_file >> numObj;
		objects.reserve(numObj);
		std::vector<std::pair<int, int>> parentChildIndex;
		parentChildIndex.reserve(numObj);

		for (int i = 0; i < numObj; i++) {
			read_file >> prefix;

			std::string objName;
			std::string objType;

			read_file >> prefix;
			read_file >> objName;
			read_file >> prefix;
			read_file >> prefix;
			read_file >> objType;

			ShrPtr<Object> object;
			if (objType == ObjectType2Name(ObjectType::INSTANCE)) {
				std::shared_ptr<Instance> instance = MakeShr<Instance>();
				uint32_t index;
				read_file >> prefix;
				read_file >> index;
				instance->geometryIndex = index;
				object = instance;
			}
			else if (objType == ObjectType2Name(ObjectType::CAMERA)) {
				std::shared_ptr<CameraObject> camera = MakeShr<CameraObject>();
				read_file >> prefix;
				read_file >> camera->yFov;

				object = camera;
			}
			else {
				SKHOLE_UNIMPL();
			}

			object->objectName = objName;

			//Transform
			read_file >> prefix;
			read_file >> prefix;
			read_file >> object->localTranslation.v[0] >> object->localTranslation.v[1] >> object->localTranslation.v[2];
			read_file >> prefix;
			read_file >> object->localQuaternion.x >> object->localQuaternion.y >> object->localQuaternion.y >> object->localQuaternion.z;
			read_file >> prefix;
			read_file >> object->localScale.v[0] >> object->localScale.v[1] >> object->localScale.v[2];

			read_file >> prefix;
			read_file >> prefix;
			int parentIndex;
			read_file >> parentIndex;
			read_file >> prefix;
			int childIndex;
			read_file >> childIndex;

			parentChildIndex.push_back({ parentIndex, childIndex });

			//Animation
			read_file >> prefix;
			bool useAnimation;
			read_file >> useAnimation;
			object->useAnimation = useAnimation;

			if (useAnimation) {
				read_file >> prefix;
				int numAnim;
				read_file >> numAnim;

				for (int j = 0; j < numAnim; j++) {
					std::string animType;
					read_file >> animType;

					int numKey;
					read_file >> numKey;

					if (animType == "Translation") {
						for (int i = 0; i < numKey; i++)
						{
							float frame;
							vec3 value;
							read_file >> frame >> value.v[0] >> value.v[1] >> value.v[2];
							object->translationAnimation.AppendKey(KeyFrame<vec3>(value, frame));
						}
					}
					else if (animType == "Rotation") {
						for (int i = 0; i < numKey; i++)
						{
							float frame;
							Quaternion value;
							read_file >> frame >> value.x >> value.y >> value.z >> value.w;
							object->rotationAnimation.AppendKey(KeyFrame<Quaternion>(value, frame));
						}
					}
					else if (animType == "Scale") {
						for (int i = 0; i < numKey; i++)
						{
							float frame;
							vec3 value;
							read_file >> frame >> value.v[0] >> value.v[1] >> value.v[2];
							object->scaleAnimation.AppendKey(KeyFrame<vec3>(value, frame));
						}
					}
					else {
						SKHOLE_UNIMPL();
					}
				}
			}

			objects.push_back(object);
		} // object roop

		// Set Parent Child
		for (int i = 0; i < objects.size(); i++)
		{
			auto& obj = objects[i];
			auto& pcindex = parentChildIndex[i];
			if (pcindex.first != -1) {
				obj->parentObject = objects[pcindex.first];
			}
			if (pcindex.second != -1) {
				obj->childObject = objects[pcindex.second];
			}
		}

		read_file.close();

	}

	inline bool ImportGeometry(const std::string& path, std::vector<ShrPtr<Geometry>>& geometry) {
		std::ifstream read_file(path);
		if (!read_file.is_open()) {
			SKHOLE_LOG("Failed to open file : " + path);
			return false;
		}

		std::string prefix;
		read_file >> prefix;

		if (prefix != "Geometries") return false;

		int numGeom;
		read_file >> numGeom;

		geometry.reserve(numGeom);

		for (int i = 0; i < numGeom; i++) {
			ShrPtr<Geometry> geom = MakeShr<Geometry>();
			read_file >> prefix;

			// Vertices
			read_file >> prefix;
			if (prefix != "#Vertices") return false;
			int numVert;
			read_file >> numVert;
			geom->m_vertices.reserve(numVert);

			for (int j = 0; j < numVert; j++) {
				VertexData vert;
				read_file >> prefix;

				read_file >> vert.position.v[0] >> vert.position.v[1] >> vert.position.v[2] >> vert.position.v[3];

				read_file >> prefix;
				read_file >> vert.normal.v[0] >> vert.normal.v[1] >> vert.normal.v[3] >> vert.normal.v[4];

				read_file >> prefix;
				read_file >> vert.texcoord0[0] >> vert.texcoord0[1];

				read_file >> prefix;
				read_file >> vert.texcoord1[0] >> vert.texcoord1[1];

				read_file >> prefix;
				read_file >> vert.color.v[0] >> vert.color.v[1] >> vert.color.v[2] >> vert.color.v[3];

				geom->m_vertices.push_back(vert);
			}

			//Indices
			read_file >> prefix;
			if (prefix != "#Indices") return false;
			int numIndex;
			read_file >> numIndex;
			geom->m_indices.reserve(numIndex);

			for (int j = 0; j < numIndex; j++) {
				uint32_t index;
				read_file >> index;
				geom->m_indices.push_back(index);
			}

			//MaterialIndices
			read_file >> prefix;
			if (prefix != "#MaterialIndices") return false;
			int numMatIndex;
			read_file >> numMatIndex;
			geom->m_materialIndices.reserve(numMatIndex);

			for (int j = 0; j < numMatIndex; j++) {
				uint32_t index;
				read_file >> index;
				geom->m_materialIndices.push_back(index);
			}

			geometry.push_back(geom);
		}

		read_file.close();
	}

	inline ImportSettingOutput ImportSetting(std::string& path, std::string& file) {

		nlohmann::json loadJs;
		std::ifstream read_file(path + file);
		if (!read_file.is_open()) {
			SKHOLE_LOG("Failed to open file : " + path);
			return { false, nullptr, {} };
		}

		std::string filename;
		filename = DeleteFileExtension(file);

		std::string geomExt = ".skgeom";
		std::string objExt = ".skobj";

		read_file >> loadJs;

		read_file.close();

		// Offline Rendering Info
		auto& offlineInfo = loadJs["OfflineRenderingInfo"];
		OfflineRenderingInfo offlineRenderingInfo;
		offlineRenderingInfo.width = offlineInfo["Width"];
		offlineRenderingInfo.height = offlineInfo["Height"];
		offlineRenderingInfo.spp = offlineInfo["SPP"];
		offlineRenderingInfo.startFrame = offlineInfo["StartFrame"];
		offlineRenderingInfo.endFrame = offlineInfo["EndFrame"];
		offlineRenderingInfo.fps = offlineInfo["fps"];
		offlineRenderingInfo.useLimitTime = offlineInfo["Use LimitTime"];
		offlineRenderingInfo.limitTime = offlineInfo["Limit Time"];
		offlineRenderingInfo.filename = offlineInfo["Filename"];
		offlineRenderingInfo.filepath = offlineInfo["Filepath"];

		// Scene
		auto& sceneJs = loadJs["Scene"];
		auto scene = MakeShr<Scene>();
		scene->m_scenenName = sceneJs["SceneName"];

		// Renderer Parameter

		// Geometry
		std::string geomPath = path + filename + geomExt;
		ImportGeometry(geomPath, scene->m_geometies);

		// Instance
		std::string objPath = path + filename + objExt;
		ImportObjects(objPath, scene->m_objects);

		return { true, scene, offlineRenderingInfo };
	}
}



