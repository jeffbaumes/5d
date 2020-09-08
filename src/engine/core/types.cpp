#include "types.hpp"

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
