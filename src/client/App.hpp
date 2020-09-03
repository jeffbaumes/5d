#ifndef APP_HPP
#define APP_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <chrono>
#include <map>
#include <vector>

#include "../common/vec5.hpp"
#include "VulkanUtil.hpp"
#include "World.hpp"

class WorldClient;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const float ANIMATION_TIME = 2.0f;

typedef std::array<int, 6> SideIndex;

class App {
   public:
    App();
    App(WorldClient *client);
    ~App();

    void run();

    World *world;

   private:
    GLFWwindow *window;
    VulkanUtil vulkan;
    std::vector<int> cells;
    std::map<SideIndex, size_t> sideIndices;

    int buildMat = 1;
    bool cursorLocked = false;
    bool firstMousePosition = true;
    bool framebufferResized = false;
    double lastX = 0;
    double lastY = 0;

    void initWindow();
    void initVulkan();
    void initWorld();
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos);
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void mainLoop();
    void cleanup();
    void createSurface();

    // Player stuff
    float forwardVel = 0.0f;
    float backVel = 0.0f;
    float upVel = 0.0f;
    float downVel = 0.0f;
    float leftVel = 0.0f;
    float rightVel = 0.0f;
    bool uvTravel = false;
    float fallVel = 0.0f;
    float walkVel = 2.0f;
    bool flying = true;
    vec5 location = {
        CHUNK_SIZE_XZUV / 2 + 0.5f,
        CHUNK_SIZE_Y / 2 + 3,
        CHUNK_SIZE_XZUV / 2 + 0.5f,
        CHUNK_SIZE_XZUV / 2 + 0.5f,
        CHUNK_SIZE_XZUV / 2 + 0.5f};
    glm::vec3 lookHeading = glm::vec3(1.0f, 0.0f, 0.0f);
    float lookAltitude = 0.0f;
    float height = 1.75f;
    float radius = 0.25f;
    bool holdingJump = false;
    bool inJump = false;
    CellLoc focusedCell;
    CellLoc buildCell;
    float uvView = 0.0f;
    float uvViewTarget = 0.0f;

    glm::vec3 lookDir();
    void swivel(float deltaX, float deltaY);
    void collide(float h, int x, int y, int z, int u, int v);

    void updatePosition(float time);

    std::chrono::high_resolution_clock::time_point lastTime;
    bool firstTime = true;

    void updateUniforms();
};

#endif // APP_HPP
