#pragma once

#include <include.h>
#include <common/filepath.h>
#include <scene/scene.h>
#include <loader/obj_loader.h>

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

				return loadScene;
			}
			else {
				SKHOLE_UNIMPL();
			}
		}
	};
}
