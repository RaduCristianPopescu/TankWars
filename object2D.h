#pragma once

#include <string>

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"

//#include "components/simple_scene.h"


namespace object2D
{
    Mesh* CreateSquare(const std::string& name, glm::vec3 leftBottomCorner, float length, glm::vec3 color, bool fill = false);
	Mesh* CreateTrapezoid(const std::string& name, glm::vec3 color);
	Mesh* CreateCircle(const std::string& name, glm::vec3 color, float radius, int numSegments);
	Mesh* CreateRectangle(const std::string& name, glm::vec3 color);
}
