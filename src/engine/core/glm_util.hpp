#pragma once

#include "glm_include.hpp"

glm::vec3 project(glm::vec3 a, glm::vec3 b);

// ProjectToPlane projects a vector onto a plane with a given normal
glm::vec3 projectToPlane(glm::vec3 v, glm::vec3 n);
