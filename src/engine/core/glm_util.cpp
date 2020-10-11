#include "glm_util.hpp"

glm::vec3 project(glm::vec3 a, glm::vec3 b) {
    glm::vec3 bn = glm::normalize(b);
    return bn * glm::dot(a, bn);
}

// ProjectToPlane projects a vector onto a plane with a given normal
glm::vec3 projectToPlane(glm::vec3 v, glm::vec3 n) {
    if (glm::length(v) == 0) {
        return v;
    }
    // To project vector to plane, subtract vector projected to normal
    return v - project(v, n);
}
