#include "types.hpp"
#include "World.hpp"

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

SurroundingCells::SurroundingCells(World &world, CellLoc loc) {
    int x = loc.x;
    int y = loc.y;
    int z = loc.z;
    int u = loc.u;
    int v = loc.v;

    negativeX = CellWithLoc(world, {x - 1, y, z, u, v});
    negativeU = CellWithLoc(world, {x, y, z, u - 1, v});
    positiveX = CellWithLoc(world, {x + 1, y, z, u, v});
    positiveU = CellWithLoc(world, {x, y, z, u + 1, v});
    negativeZ = CellWithLoc(world, {x, y, z - 1, u, v});
    negativeV = CellWithLoc(world, {x, y, z, u, v - 1});
    positiveZ = CellWithLoc(world, {x, y, z + 1, u, v});
    positiveV = CellWithLoc(world, {x, y, z, u, v + 1});
    negativeY = CellWithLoc(world, {x, y - 1, z, u, v});
    positiveY = CellWithLoc(world, {x, y + 1, z, u, v});
}

CellWithLoc::CellWithLoc() {
    cell = 0;
    loc = {0, 0, 0, 0, 0};
}

CellWithLoc::CellWithLoc(Cell cell, CellLoc loc) {
    this->cell = cell;
    this->loc = loc;
}

CellWithLoc::CellWithLoc(World &world, CellLoc loc) {
    this->loc = loc;
    this->cell = world.getCell(loc);
}