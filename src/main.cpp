#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "VulkanUtil.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class App {
   public:
    void run() {
        initWindow();
        initVulkan();
        createSurface();
        addWorldVertices();
        mainLoop();
        cleanup();
    }

   private:
    GLFWwindow *window;

    VulkanUtil vulkan;

    std::vector<int> cells;

    std::vector<Vertex> vertices;

    std::vector<uint32_t> indices;

    bool cursorLocked = false;
    bool firstMousePosition = true;

    bool framebufferResized = false;

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPositionCallback);
    }

    void initVulkan() {
        vulkan.init();
    }

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
        app->vulkan.framebufferResized = true;
    }

    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
        auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            app->cursorLocked = true;
            app->firstMousePosition = true;
        }
    }

    double lastX = 0;
    double lastY = 0;

    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
        auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
        if (app->cursorLocked) {
            if (app->firstMousePosition) {
                app->lastX = xpos;
                app->lastY = ypos;
                app->firstMousePosition = false;
            }
            app->swivel(xpos - app->lastX, ypos - app->lastY);
            app->lastX = xpos;
            app->lastY = ypos;
        }
    }


    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_W) {
                app->forwardVel = app->walkVel;
            } else if (key == GLFW_KEY_S) {
                app->backVel = app->walkVel;
            } else if (key == GLFW_KEY_D) {
                app->rightVel = app->walkVel;
            } else if (key == GLFW_KEY_A) {
                app->leftVel = app->walkVel;
            } else if (key == GLFW_KEY_SPACE) {
                app->holdingJump = true;
            } else if (key == GLFW_KEY_LEFT_SHIFT) {
                app->uvTravel = true;
                app->oldUv = app->uv;
            } else if (key == GLFW_KEY_ESCAPE) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                app->cursorLocked = false;
            }
        } else if (action == GLFW_RELEASE) {
            if (key == GLFW_KEY_W) {
                app->forwardVel = 0.0f;
            } else if (key == GLFW_KEY_S) {
                app->backVel = 0.0f;
            } else if (key == GLFW_KEY_D) {
                app->rightVel = 0.0f;
            } else if (key == GLFW_KEY_A) {
                app->leftVel = 0.0f;
            } else if (key == GLFW_KEY_SPACE) {
                app->holdingJump = false;
                app->upVel = 0.0f;
            } else if (key == GLFW_KEY_LEFT_SHIFT) {
                app->uvTravel = false;
                glm::vec2 uvDist = app->uv - app->oldUv;
                app->uv = glm::floor(app->oldUv) + glm::floor(uvDist);
            }
        }
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            updateUniforms();
            vulkan.draw();
        }
    }

    void cleanup() {
        vulkan.cleanup();

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void createSurface() {
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(vulkan.instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        vulkan.initSurface(surface);
    }

    void addVertex(const Vertex &vertex, std::unordered_map<Vertex, uint32_t> &uniqueVertices) {
        if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
        }
        indices.push_back(uniqueVertices[vertex]);
    }

    void generateCube(int x, int y, int z, int u, int v, std::unordered_map<Vertex, uint32_t> &uniqueVertices) {

        addVertex({{0, 0, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 1, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 0, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{0, 0, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{0, 1, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 1, 0}, {x, y, z}, {u, v}}, uniqueVertices);

        addVertex({{0, 0, 1}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 0, 1}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 1, 1}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{0, 0, 1}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 1, 1}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{0, 1, 1}, {x, y, z}, {u, v}}, uniqueVertices);

        addVertex({{0, 0, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{0, 1, 1}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{0, 1, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{0, 0, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{0, 0, 1}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{0, 1, 1}, {x, y, z}, {u, v}}, uniqueVertices);

        addVertex({{1, 0, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 1, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 1, 1}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 0, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 1, 1}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 0, 1}, {x, y, z}, {u, v}}, uniqueVertices);

        addVertex({{0, 0, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 0, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 0, 1}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{0, 0, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 0, 1}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{0, 0, 1}, {x, y, z}, {u, v}}, uniqueVertices);

        addVertex({{0, 1, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 1, 1}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 1, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{0, 1, 0}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{0, 1, 1}, {x, y, z}, {u, v}}, uniqueVertices);
        addVertex({{1, 1, 1}, {x, y, z}, {u, v}}, uniqueVertices);
    }

    void setCell(int x, int y, int z, int u, int v, int material) {
        if (x < 0 || x >= size || y < 0 || y >= size || z < 0 || z >= size || u < 0 || u >= size || v < 0 || v >= size) {
            return;
        }
        cells[x + size * y + size * size * z + size * size * size * u + size * size * size * size * v] = material;
    }

    int getCell(int x, int y, int z, int u, int v) {
        if (x < 0 || x >= size || y < 0 || y >= size || z < 0 || z >= size || u < 0 || u >= size || v < 0 || v >= size) {
            return 0;
        }
        return cells[x + size * y + size * size * z + size * size * size * u + size * size * size * size * v];
    }

    int size = 8;

    void addWorldVertices() {
        cells.resize(size * size * size * size * size, 0);
        std::unordered_map<Vertex, uint32_t> uniqueVertices;
        for (int x = 0; x < size; x += 1) {
            for (int y = 0; y < size; y += 1) {
                for (int z = 0; z < size; z += 1) {
                    for (int u = 0; u < size; u += 1) {
                        for (int v = 0; v < size; v += 1) {
                            int dx = x - size / 2;
                            int dy = y - size / 2;
                            int dz = z - size / 2;
                            int du = u - size / 2;
                            int dv = v - size / 2;
                            // if (dx * dx + dy * dy + dz * dz + du * du + dv * dv < (size / 2) * (size / 2)) {
                            // int s = size / 4;
                            // if (x <= s && x >= -s && y <= s && y >= -s && z <= s && z >= -s && u <= s && u >= -s && v <= s && v >= -s) {
                            // if (y <= size / 2 || x == 0 || y == 0 || z == 0 || u == 0 || v == 0) {
                            if (x / 2 + y + z / 2 + u / 2 + v / 2 < 10) {
                                generateCube(x, y, z, u, v, uniqueVertices);
                                setCell(x, y, z, u, v, 1);
                            }
                        }
                    }
                }
            }
        }

        vulkan.setVerticesAndIndices(vertices, indices);
    }

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
    glm::vec3 loc = glm::vec3(size / 2, size / 2 + 2, size / 2);
    glm::vec2 uv = glm::vec2(size / 2, size / 2);
    glm::vec3 lookHeading = glm::vec3(1.0f, 0.0f, 0.0f);
    float lookAltitude = 0.0f;
    float height = 1.0f;
    float radius = 0.1f;
    glm::vec2 oldUv = glm::vec2(size / 2, size / 2);
    bool holdingJump = false;
    bool inJump = false;

    glm::vec3 project(glm::vec3 a, glm::vec3 b) {
        glm::vec3 bn = glm::normalize(b);
        return bn * glm::dot(a, bn);
    }

    // ProjectToPlane projects a vector onto a plane with a given normal
    glm::vec3 projectToPlane(glm::vec3 v, glm::vec3 n) {
        if (glm::length(v) == 0) {
            return v;
        }
        // To project vector to plane, subtract vector projected to normal
        return v - project(v, n);
    }

    glm::vec3 lookDir() {
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        lookHeading = glm::normalize(projectToPlane(lookHeading, up));
        glm::vec3 right = glm::cross(lookHeading, up);
        return glm::rotate(glm::mat4(1.0f), glm::radians(lookAltitude - 90.0f), right) * glm::vec4(up, 1.0f);
    }

    void swivel(float deltaX, float deltaY) {
        float lookHeadingDelta = -0.1f * deltaX;
        glm::vec3 normalDir(0.0f, 1.0f, 0.0f);
        lookHeading = glm::rotate(glm::mat4(1.0f), glm::radians(lookHeadingDelta), normalDir) * glm::vec4(lookHeading, 1.0f);
        lookAltitude = lookAltitude - 0.1f * deltaY;
        lookAltitude = std::max(std::min(lookAltitude, 89.9f), -89.9f);
    }

    void collide(float h, int x, int y, int z, int u, int v) {
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        glm::vec3 pos = loc - (up * (height - h));
        glm::vec3 d = glm::vec3(x, y, z);
        glm::vec2 dUV = glm::vec2(u, v);
        glm::vec3 cell = glm::floor(pos) + glm::vec3(0.5f, 0.5f, 0.5f);
        glm::vec2 cellUV = glm::floor(uv) + glm::vec2(0.5f, 0.5f);
        glm::vec3 adjCell = cell + d;
        glm::vec2 adjCellUV = cellUV + dUV;
        int material = getCell(adjCell.x, adjCell.y, adjCell.z, adjCellUV.x, adjCellUV.y);
        if (material != 0) {
            if (y != 0) {
                // TODO
            } else {
                if (u != 0 || v != 0) {
                    cell.x = cellUV.x;
                    cell.z = cellUV.y;
                    d.x = dUV.x;
                    d.z = dUV.y;
                    pos.x = uv.x;
                    pos.z = uv.y;
                }
                glm::vec3 nLoc = cell + d / 2.0f;
                glm::vec3 aLoc = cell + d;
                glm::vec3 cNorm = glm::normalize(nLoc - aLoc);
                cNorm = glm::normalize(cNorm - project(cNorm, up));
                float dist2Plane = glm::dot(cNorm, pos - nLoc);
                if (dist2Plane < radius) {
                    float move = radius - dist2Plane;
                    cNorm = cNorm * move;
                    if (u != 0 || v != 0) {
                        if (uvTravel) {
                            uv = uv + glm::vec2(cNorm.x, cNorm.z);
                        }
                        loc.y = loc.y + cNorm.y;
                    } else {
                        loc = loc + cNorm;
                    }
                }
            }
        }
    }

    void updatePosition(float time) {
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::cross(lookHeading, up);
        glm::vec3 feet = loc - (up * height);
        int feetCell = getCell(floor(feet.x), floor(feet.y), floor(feet.z), floor(uv.x), floor(uv.y));
        bool falling = feetCell == 0;
        // bool falling = feet.y > size / 2;
        if (falling) {
            fallVel -= 20.0f * time;
        } else if (holdingJump && !inJump) {
            fallVel = 7.0f;
            inJump = true;
        } else {
            fallVel = 0.0f;
            inJump = false;
        }

        glm::vec3 playerVel = up * fallVel;
        playerVel = playerVel + lookHeading * (forwardVel - backVel);
        playerVel = playerVel + right * (rightVel - leftVel);
        if (uvTravel) {
            uv = uv + (glm::vec2(playerVel.x, playerVel.z) * time);
            loc.y = loc.y + (playerVel.y * time);
        } else {
            loc = loc + (playerVel * time);
        }
        for (float h = 0.5f; h < height; h += 1.0f) {
            collide(h, 1, 0, 0, 0, 0);
            collide(h, -1, 0, 0, 0, 0);
            // collide(h, 0, 1, 0, 0, 0);
            // collide(h, 0, -1, 0, 0, 0);
            collide(h, 0, 0, 1, 0, 0);
            collide(h, 0, 0, -1, 0, 0);
            collide(h, 0, 0, 0, 1, 0);
            collide(h, 0, 0, 0, -1, 0);
            collide(h, 0, 0, 0, 0, 1);
            collide(h, 0, 0, 0, 0, -1);
        }


        std::cout << loc.x << "," << loc.y << "," << loc.z << std::endl;
        std::cout << oldUv.x << "," << oldUv.y << std::endl;
        std::cout << uv.x << "," << uv.y << std::endl;


        // TODO: Update focused cell here
    }

    std::chrono::high_resolution_clock::time_point lastTime;
    bool firstTime = true;

    void updateUniforms() {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        if (firstTime) {
            lastTime = currentTime;
            firstTime = false;
        }
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
        lastTime = currentTime;

        updatePosition(timeDelta);

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), 0.0f * time * glm::radians(90.0f) / 4.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::vec3 look = lookDir();
        ubo.view = glm::lookAt(loc, loc + look, glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), vulkan.swapChainExtent.width / (float)vulkan.swapChainExtent.height, 0.1f, 50.0f);
        ubo.proj[1][1] *= -1;

        ubo.uv = uv;

        vulkan.ubo = ubo;
    }
};

int main() {
    App app;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
