#ifndef D5_ENTITY_H_
#define D5_ENTITY_H_

#include "VulkanUtil.hpp"
#include "vec5.hpp"
#include <deque>

class World;

class Entity {
   public:
    vec5 location;
    glm::vec3 rotation;

    void updateUBO(UniformBufferObject *ubo);

    void setMaterial(int material);
    const int getMaterial() const;

    const int & id() const;

    Entity(World *world, int material);

    void destroy();

    void init();
    void init(World *worldIn, int material);

   private:
    size_t indexLocation;
    size_t vertexLocation;
    int _material;
    int _id;
    World *world;

    void show();

    void hide();
};
#endif
