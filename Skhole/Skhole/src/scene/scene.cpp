#include <scene/scene.h>
#include <scene/object/sample_geometry.h>


namespace Skhole {


	Scene::Scene() {
	}

	Scene::~Scene() {
	}

	void Scene::Initialize() {
		ShrPtr<Geometry> geometry = MakeShr<Geometry>(BoxGeometry());
		ShrPtr<Instance> object = MakeShr<Instance>();
		object->m_instanceName = "Instance 0";
		object->m_geometryIndex = 0;

		ShrPtr<BasicMaterial> material = MakeShr<BasicMaterial>();
		material->basecolor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		material->metallic = 0.0f;
		material->roughness = 0.5f;
		material->emissionColor = vec3(0.0f, 0.0f, 0.0f);
		material->emissionIntensity = 0.0f;

		m_geometies.push_back(geometry);
		m_objects.push_back(object);
		m_basicMaterials.push_back(material);
	}

	void Scene::RendererSet(const std::shared_ptr<Renderer>& renderer) {
		for (auto& mat : m_basicMaterials)
		{
			m_materials.push_back(renderer->GetMaterial(mat));
		}
	}

	void Scene::LoadModelFile() {
		SKHOLE_UNIMPL();	
	}

	void Scene::LoadScene(std::string path) {
		SKHOLE_UNIMPL();
	}
	void Scene::SaveScene(std::string path) {
		SKHOLE_UNIMPL();	
	}

}