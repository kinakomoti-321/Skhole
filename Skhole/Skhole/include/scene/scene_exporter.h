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

	bool ExportMaterials(const std::string& filepath, const std::string& filename, nlohmann::json& matJs)
	{
		return true;
	}

	bool ExportTextures();

	inline bool ExportScene(const std::string& filepath, const std::string& filename, const ShrPtr<Scene>& scene, nlohmann::json& sJs) {
		auto& objects = scene->m_objects;
		auto& geometries = scene->m_geometies;
		auto& materials = scene->m_materials;
		auto& basicMaterials = scene->m_basicMaterials;
		auto& textures = scene->m_textures;


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

		////Materials

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



