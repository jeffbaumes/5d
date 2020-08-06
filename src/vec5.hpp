#ifndef D5_VEC5_H
#define D5_VEC5_H
#include <glm/glm.hpp>

struct vec5 {
    float x;
    float y;
    float z;
    float u;
    float v;
    vec5 operator+(const vec5 &a) {
        return {x + a.x, y + a.y, z + a.z, u + a.u, v + a.v};
    }
    vec5 operator+(float m) {
        return {x + m, y + m, z + m, u + m, v + m};
    }
    vec5 operator-(float m) {
        return {x - m, y - m, z - m, u - m, v - m};
    }
    vec5 operator-(const vec5 &a) {
        return {x - a.x, y - a.y, z - a.z, u - a.u, v - a.v};
    }
    vec5 operator*(float m) {
        return {m * x, m * y, m * z, m * u, m * v};
    }
    vec5 operator/(float m) {
        return {x / m, y / m, z / m, u / m, v / m};
    }
    glm::vec3 xyz() {
        return {x, y, z};
    }
    glm::vec2 uv() {
        return {u, v};
    }
    void print() {
        std::cerr << x << "," << y << "," << z << "," << u << "," << v << " " << std::endl;
    }
};

#endif
