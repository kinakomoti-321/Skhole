#include <scene/scene.h>
#include <scene/object/sample_geometry.h>
#include <common/math.h>


namespace Skhole {


	Scene::Scene() {
	}

	Scene::~Scene() {
	}

	void Scene::Initialize() {
		ShrPtr<Geometry> geometry = MakeShr<Geometry>();
		Geometry geom = BoxGeometry();
		geometry->m_vertices = geom.m_vertices;
		geometry->m_indices = geom.m_indices;
		geometry->m_materialIndices = geom.m_materialIndices;

		ShrPtr<Geometry> geometry2 = MakeShr<Geometry>();
		geometry2->m_vertices = geom.m_vertices;
		geometry2->m_indices = geom.m_indices;
		geometry2->m_materialIndices = geom.m_materialIndices;

		ShrPtr<Instance> object = MakeShr<Instance>();
		object->objectName = "Instance";
		object->geometryIndex = 0;
		object->localPosition = vec3(0.0f, 0.0f, 0.0f);
		object->localRotationEular = vec3(0.0f, 0.0f, 0.0f);
		object->localScale = vec3(1.0f, 1.0f, 1.0f);

		ShrPtr<Instance> object2 = MakeShr<Instance>();
		object2->objectName = "Instance2";
		object2->geometryIndex = 0;
		object2->localPosition = vec3(0.0f, 0.0f, -10.0f);
		object2->localRotationEular = vec3(0.0f, 0.0f, 0.0f);
		object2->localScale = vec3(1.0f, 1.0f, 1.0f);

		ShrPtr<Instance> object3 = MakeShr<Instance>();
		object3->objectName = "Instance3";
		object3->geometryIndex = 1;
		object3->localPosition = vec3(0.0f, 0.0f, 10.0f);
		object3->localRotationEular = vec3(0.0f, 0.0f, 0.0f);
		object3->localScale = vec3(1.0f, 1.0f, 1.0f);

		ShrPtr<BasicMaterial> material = MakeShr<BasicMaterial>();
		material->basecolor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		material->metallic = 0.0f;
		material->roughness = 0.5f;
		material->emissionColor = vec3(0.0f, 0.0f, 0.0f);
		material->emissionIntensity = 0.0f;

		m_geometies.push_back(geometry);
		m_geometies.push_back(geometry2);
		m_objects.push_back(object);
		m_objects.push_back(object2);
		m_objects.push_back(object3);

		m_basicMaterials.push_back(material);
	}

	void Scene::RendererSet(const std::shared_ptr<Renderer>& renderer) {
		for (auto& mat : m_basicMaterials)
		{
			m_materials.push_back(renderer->GetMaterial(mat));
		}

		m_basicCamera = MakeShr<BasicCamera>();

		m_camera = renderer->GetCamera(m_basicCamera);
	}


	void Scene::SetTransformMatrix(float frame){
		for (auto& object : m_objects) {
			object->SetWorldTransformMatrix(frame);
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