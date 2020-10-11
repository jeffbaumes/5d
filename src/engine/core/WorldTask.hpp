#pragma once

class World;

class WorldTask {
public:
    virtual ~WorldTask() = default;
    virtual void executeTask(World &world, float dt) = 0;
};
