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

    std::vector<Vertex> vertices;

    std::vector<uint32_t> indices;

    bool framebufferResized = false;

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetKeyCallback(window, keyCallback);
    }

    void initVulkan() {
        vulkan.init();
    }

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
        app->vulkan.framebufferResized = true;
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

    void addWorldVertices() {
        int size = 8;
        std::unordered_map<Vertex, uint32_t> uniqueVertices;
        for (int x = -size / 2; x < size / 2; x += 1) {
            for (int y = -size / 2; y < size / 2; y += 1) {
                for (int z = -size / 2; z < size / 2; z += 1) {
                    for (int u = -size / 2; u < size / 2; u += 1) {
                        for (int v = -size / 2; v < size / 2; v += 1) {
                            // if (x * x + y * y + z * z + u * u + v * v < (size / 2) * (size / 2))
                            // int s = size / 4;
                            // if (x <= s && x >= -s && y <= s && y >= -s && z <= s && z >= -s && u <= s && u >= -s && v <= s && v >= -s) {
                            if (y <= 0) {
                                generateCube(x, y, z, u, v, uniqueVertices);
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
    float walkVel = 5.0f;
    glm::vec3 loc = glm::vec3(0.0f, 2.0f, 0.0f);
    glm::vec3 lookHeading = glm::vec3(1.0f, 0.0f, 0.0f);
    float lookAltitude = 0.0f;
    float height = 2.0f;
    float radius = 0.25f;
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
        // std::cout << lookHeading.x << "," << lookHeading.y << "," << lookHeading.z << std::endl;
        lookHeading = glm::normalize(projectToPlane(lookHeading, up));
        glm::vec3 right = glm::cross(lookHeading, up);
        return glm::rotate(glm::mat4(1.0f), glm::radians(lookAltitude - 90.0f), right) * glm::vec4(up, 1.0f);
    }

    void swivel(float deltaX, float deltaY) {
        float lookHeadingDelta = -0.1f * deltaX;
        glm::vec3 normalDir(0.0f, 1.0f, 0.0f);
        lookHeading = glm::rotate(glm::mat4(1.0f), glm::radians(lookHeadingDelta), normalDir) * glm::vec4(lookHeading, 1.0f);
        lookAltitude = lookAltitude - 0.1f * deltaY;
    }

    void updatePosition(float time) {
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::cross(lookHeading, up);
        glm::vec3 feet = loc - (up * height);
        // Cell feetCell = cartesianToCell(feet);
        // bool falling = feetCell.material == 0;
        bool falling = feet.y > 0.0f;
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
        std::cout << playerVel.x << "," << playerVel.y << "," << playerVel.z << std::endl;
        std::cout << time;
        loc = loc + (playerVel * time);

        // TODO: Update focused cell here
    }

    std::chrono::system_clock::time_point lastTime;

    void updateUniforms() {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
        lastTime = currentTime;

        std::cerr << timeDelta << std::endl;

        updatePosition(timeDelta);

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), 0.0f * time * glm::radians(90.0f) / 4.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        // ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // ubo.view = glm::lookAt(glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // std::cout << loc.x << "," << loc.y << "," << loc.z << std::endl;
        glm::vec3 look = lookDir();
        // std::cout << look.x << "," << look.y << "," << look.z << std::endl;
        ubo.view = glm::lookAt(loc, loc + look, glm::vec3(0.0f, 1.0f, 0.0f));
        // ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj = glm::perspective(glm::radians(45.0f), vulkan.swapChainExtent.width / (float)vulkan.swapChainExtent.height, 0.1f, 50.0f);
        ubo.proj[1][1] *= -1;

        ubo.uv = glm::vec2(2.0f * glm::sin(time / 4.0f), 2.0f * glm::sin(time / 4.0f));

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
