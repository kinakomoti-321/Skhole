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
		Geometry geom = PlaneGeometry(vec3(0.0, 0.0, -4.0), vec3(0.0, 4.0, 0.0), vec3(-2.0, 0.0, 0.0), 0); // Left
		geometry->m_vertices = geom.m_vertices;
		geometry->m_indices = geom.m_indices;
		geometry->m_materialIndices = geom.m_materialIndices;

		ShrPtr<Geometry> geometry2 = MakeShr<Geometry>();
		geom = PlaneGeometry(vec3(0.0, 4.0, 0.0), vec3(0.0, 0.0, -4.0), vec3(2.0, 0.0, 0.0), 1); // Right
		geometry2->m_vertices = geom.m_vertices;
		geometry2->m_indices = geom.m_indices;
		geometry2->m_materialIndices = geom.m_materialIndices;

		ShrPtr<Geometry> geometry3 = MakeShr<Geometry>();
		geom = PlaneGeometry(vec3(4.0, 0.0, 0.0), vec3(0.0, 0.0, -4.0), vec3(-2.0, 0.0, 0.0), 2); // Bottom
		geometry3->m_vertices = geom.m_vertices;
		geometry3->m_indices = geom.m_indices;
		geometry3->m_materialIndices = geom.m_materialIndices;

		ShrPtr<Geometry> geometry4 = MakeShr<Geometry>();
		geom = PlaneGeometry(vec3(0.0, 0.0, -4.0), vec3(4.0, 0.0, 0.0), vec3(-2.0, 4.0, 0.0), 2); // Top
		geometry4->m_vertices = geom.m_vertices;
		geometry4->m_indices = geom.m_indices;
		geometry4->m_materialIndices = geom.m_materialIndices;

		ShrPtr<Geometry> geometry5 = MakeShr<Geometry>();
		geom = PlaneGeometry(vec3(4.0, 0.0, 0.0), vec3(0.0, 4.0, 0.0), vec3(-2.0, 0.0, -4.0), 2); // Back
		geometry5->m_vertices = geom.m_vertices;
		geometry5->m_indices = geom.m_indices;
		geometry5->m_materialIndices = geom.m_materialIndices;

		ShrPtr<Geometry> geometry6 = MakeShr<Geometry>();
		geom = PlaneGeometry(vec3(0.0, 0.0, 2.0), vec3(2.0, 0.0, 0.0), vec3(-1.0, 3.9, -3.0), 3); // Light
		geometry6->m_vertices = geom.m_vertices;
		geometry6->m_indices = geom.m_indices;
		geometry6->m_materialIndices = geom.m_materialIndices;

		ShrPtr<Instance> object = MakeShr<Instance>();
		object->objectName = "Instance";
		object->geometryIndex = 0;

		ShrPtr<Instance> object2 = MakeShr<Instance>();
		object2->objectName = "Instance2";
		object2->geometryIndex = 1;

		ShrPtr<Instance> object3 = MakeShr<Instance>();
		object3->objectName = "Instance3";
		object3->geometryIndex = 2;

		ShrPtr<Instance> object4 = MakeShr<Instance>();
		object4->objectName = "Instance4";
		object4->geometryIndex = 3;

		ShrPtr<Instance> object5 = MakeShr<Instance>();
		object5->objectName = "Instance4";
		object5->geometryIndex = 4;

		ShrPtr<Instance> object6 = MakeShr<Instance>();
		object6->objectName = "Instance4";
		object6->geometryIndex = 5;

		ShrPtr<BasicMaterial> material = MakeShr<BasicMaterial>();
		material->basecolor = vec4(1.0f, 0.2f, 0.2f, 1.0f);
		material->metallic = 0.0f;
		material->roughness = 0.5f;
		material->emissionColor = vec4(0.0f);
		material->emissionIntensity = 0.0f;

		ShrPtr<BasicMaterial> material2 = MakeShr<BasicMaterial>();
		material2->basecolor = vec4(0.2f, 1.0f, 0.2f, 1.0f);
		material2->metallic = 0.0f;
		material2->roughness = 0.5f;
		material2->emissionColor = vec4(0.0f);
		material2->emissionIntensity = 0.0f;

		ShrPtr<BasicMaterial> material3 = MakeShr<BasicMaterial>();
		material3->basecolor = vec4(1.0f);
		material3->metallic = 0.0f;
		material3->roughness = 0.5f;
		material3->emissionColor = vec4(0.0);
		material3->emissionIntensity = 0.0f;

		ShrPtr<BasicMaterial> material4 = MakeShr<BasicMaterial>();
		material4->basecolor = vec4(1.0f);
		material4->metallic = 0.0f;
		material4->roughness = 0.5f;
		material4->emissionColor = vec4(1.0f);
		material4->emissionIntensity = 4.0f;

		m_geometies.push_back(geometry);
		m_geometies.push_back(geometry2);
		m_geometies.push_back(geometry3);
		m_geometies.push_back(geometry4);
		m_geometies.push_back(geometry5);
		m_geometies.push_back(geometry6);

		m_objects.push_back(object);
		m_objects.push_back(object2);
		m_objects.push_back(object3);
		m_objects.push_back(object4);
		m_objects.push_back(object5);
		m_objects.push_back(object6);

		m_basicMaterials.push_back(material);
		m_basicMaterials.push_back(material2);
		m_basicMaterials.push_back(material3);
		m_basicMaterials.push_back(material4);
	}

	void Scene::RendererSet(const std::shared_ptr<Renderer>& renderer) {
		for (auto& mat : m_basicMaterials)
		{
			m_materials.push_back(renderer->GetMaterial(mat));
		}

		m_camera = MakeShr<RendererDefinisionCamera>();

		m_camera = renderer->GetCamera(m_camera);

		m_rendererParameter = renderer->GetRendererParameter();
	}


	void Scene::SetTransformMatrix(float frame) {
		for (auto& object : m_objects) {
			if (object->useAnimation) {
				object->ResetWorldTransformMatrix();
			}
		}
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