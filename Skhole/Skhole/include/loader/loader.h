#pragma once

#include <include.h>
#include <common/filepath.h>
#include <scene/scene.h>
#include <loader/obj_loader.h>
#include <loader/gltf_loader.h>
#include <scene/scene_exporter.h>

namespace Skhole {

	class Loader {
	public:
		Loader() {};
		~Loader() {};

		static ShrPtr<Scene> LoadFile(const std::string& path) {
			std::string extension;
			if (!GetFileExtension(path, extension)) {
				SKHOLE_ERROR("Invalid File Path");
			}

			ShrPtr<Scene> loadScene = MakeShr<Scene>();

			if (extension == "obj") {
				LoadObjFile(
					path,
					loadScene->m_objects,
					loadScene->m_geometies,
					loadScene->m_basicMaterials,
					loadScene->m_textures
				);

			}
			else if (extension == "glb" || extension == "gltf") {
				LoadGLTFFile(
					path,
					loadScene->m_objects,
					loadScene->m_geometies,
					loadScene->m_basicMaterials,
					loadScene->m_textures
				);
			}
			else {
				SKHOLE_UNIMPL();
			}

			// Camera Setting
			int objIndex = 0;
			for (auto& object : loadScene->m_objects) {
				if (object->GetObjectType() == ObjectType::CAMERA) {
					loadScene->m_cameraObjectIndices.push_back(objIndex);
				}
				objIndex++;
			}

			// Connect Prim Id
			for (auto& geometry : loadScene->m_geometies) {
				CreateConnectPrimId(geometry);
			}

			return loadScene;
		}

	};
}
