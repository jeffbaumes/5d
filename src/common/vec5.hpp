#ifndef VEC5_HPP
#define VEC5_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <iostream>

template <typename T>
struct vector5 {
    T x;
    T y;
    T z;
    T u;
    T v;
    vector5() {
        x = 0;
        y = 0;
        z = 0;
        u = 0;
        v = 0;
    }
    vector5(T _x, T _y, T _z, T _u, T _v) {
        x = _x;
        y = _y;
        z = _z;
        u = _u;
        v = _v;
    }
    vector5(const vector5<float> &other) {
        x = other.x;
        y = other.y;
        z = other.z;
        u = other.u;
        v = other.v;
    }
    vector5(const vector5<int> &other) {
        x = other.x;
        y = other.y;
        z = other.z;
        u = other.u;
        v = other.v;
    }
    vector5<T> operator+(const vector5<T> &a) {
        return {x + a.x, y + a.y, z + a.z, u + a.u, v + a.v};
    }
    vector5<T> operator+(T m) {
        return {x + m, y + m, z + m, u + m, v + m};
    }
    vector5<T> operator-(T m) {
        return {x - m, y - m, z - m, u - m, v - m};
    }
    vector5<T> operator-(const vector5<T> &a) {
        return {x - a.x, y - a.y, z - a.z, u - a.u, v - a.v};
    }
    vector5<T> operator*(T m) {
        return {m * x, m * y, m * z, m * u, m * v};
    }
    vector5<T> operator/(float m) {
        return {x / m, y / m, z / m, u / m, v / m};
    }
    glm::vec<3, T, glm::packed_highp> xyz() {
        return {x, y, z};
    }
    glm::vec<2, T, glm::packed_highp> uv() {
        return {u, v};
    }
    bool operator==(const vector5<T> &other) const {
        return x == other.x && y == other.y && z == other.z && u == other.u && v == other.v;
    }
    void print() const {
        std::cerr << x << "," << y << "," << z << "," << u << "," << v << " " << std::endl;
    }
    std::ostream &operator<<(std::ostream &os) {
        os << x << "," << y << "," << z << "," << u << "," << v;
        return os;
    }
};

typedef vector5<float> vec5;
typedef vector5<int> ivec5;

template <typename T>
float dot(const vector5<T> &a, const vector5<T> &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.u * b.u + a.v * b.v;
}

vec5 normalize(vec5 a);
vec5 project(vec5 a, vec5 b);
vec5 round(vec5 a);
vec5 floor(vec5 a);

glm::vec3 project(glm::vec3 a, glm::vec3 b);

// ProjectToPlane projects a vector onto a plane with a given normal
glm::vec3 projectToPlane(glm::vec3 v, glm::vec3 n);

#endif // VEC5_HPP
