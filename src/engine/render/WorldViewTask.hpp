#pragma once

class WorldView;

class WorldViewTask {
public:
    virtual ~WorldViewTask() = default;
    virtual void executeTask(WorldView &view, float dt) = 0;
};
