#pragma once

#include <include.h>

#include <scene/material/texture.h>
#include <scene/material/material.h>
#include <scene/object/object.h>
#include <scene/object/geometry.h>
#include <scene/object/instance.h>
#include <scene/camera/camera.h>


#include <renderer/renderer.h>


namespace Skhole {
	class Renderer;

	class Scene {
	public:

		Scene();
		~Scene();

		void Initialize();
		void LoadModelFile();
		void RendererSet(const std::shared_ptr<Renderer>& renderer);

		void LoadScene(std::string path);
		void SaveScene(std::string path);

		std::vector<ShrPtr<Object>> m_objects;
		std::vector<ShrPtr<Geometry>> m_geometies;
		std::vector<ShrPtr<RendererDefinisionMaterial>> m_materials;
		std::vector<ShrPtr<BasicMaterial>> m_basicMaterials;
		std::vector<ShrPtr<Texture>> m_textures;

		ShrPtr<BasicCamera> m_basicCamera;
		ShrPtr<RendererDefinisionCamera> m_camera;

		std::string m_scenenName;
	};
}
