#pragma once

#include <scene/material/texture.h>
#include <scene/material/material.h>

namespace Skhole {
	class Scene {
	public:
	private:
		std::vector<BasicMaterial> m_basicMaterials;
		std::vector<MaterialParameter> m_materials;
		std::vector<Texture> m_textures;
	};
}
