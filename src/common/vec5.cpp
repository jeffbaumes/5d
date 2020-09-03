#include "vec5.hpp"

vec5 normalize(vec5 a) {
    float len = glm::sqrt(dot(a, a));
    return a / len;
}

vec5 project(vec5 a, vec5 b) {
    vec5 bn = normalize(b);
    return bn * dot(a, bn);
}

vec5 round(vec5 a) {
    return {glm::round(a.x), glm::round(a.y), glm::round(a.z), glm::round(a.u), glm::round(a.v)};
}

vec5 floor(vec5 a) {
    return {glm::floor(a.x), glm::floor(a.y), glm::floor(a.z), glm::floor(a.u), glm::floor(a.v)};
}

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