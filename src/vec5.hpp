#ifndef D5_VEC5_H
#define D5_VEC5_H
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
    void print() {
        std::cerr << x << "," << y << "," << z << "," << u << "," << v << " " << std::endl;
    }
};

typedef vector5<float> vec5;
typedef vector5<int> ivec5;

#endif
