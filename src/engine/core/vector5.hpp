#pragma once

#include "glm_include.hpp"

#include <iostream>

template <typename T>
struct vector5 {
    T x = 0;
    T y = 0;
    T z = 0;
    T u = 0;
    T v = 0;

    vector5() = default;

    vector5<int> ivec5() {
      return {x, y, z, u, v};
    }

    vector5<float> vec5() {
      return {x, y, z, u, v};
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
};

template<typename T>
std::ostream &operator<<(std::ostream &os, const vector5<T> &v) {
    os << "(" << v.x << "," << v.y << "," << v.z << "," << v.u << "," << v.v << ")";
    return os;
}

template <typename T>
float dot(const vector5<T> &a, const vector5<T> &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.u * b.u + a.v * b.v;
}
