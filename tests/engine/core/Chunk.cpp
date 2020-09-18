#include "gtest/gtest.h"
#include "../../../src/engine/core/Chunk.hpp"

TEST(Core_Chunk, SetsACell) {
    Chunk chunk = {};
    RelativeCellLoc loc = {0, 0, 0, 0, 0};
    chunk.setCell(loc, 1);
    EXPECT_EQ(chunk.getCell(loc), 1);
}

TEST(Core_Chunk, GetsACell) {
    Chunk chunk = {};
    RelativeCellLoc loc = {0, 0, 0, 0, 0};
    EXPECT_EQ(chunk.getCell(loc), 0);
}