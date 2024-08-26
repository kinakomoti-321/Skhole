#pragma once

#include <include.h>

#include <scene/object/object.h>
#include <scene/object/geometry.h>
#include <scene/object/light.h>
#include <scene/object/instance.h>
#include <scene/object/object.h>
#include <scene/material/material.h>



namespace Skhole {
	bool LoadObjFile(
		const std::string& filename,
		std::vector<ShrPtr<Object>>& inObjects,
		std::vector<ShrPtr<Geometry>>& inGeometies,
		std::vector<ShrPtr<BasicMaterial>>& inBasicMaterials,
		std::vector<ShrPtr<Texture>>& inTextures
	);
}
