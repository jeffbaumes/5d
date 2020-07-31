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
#include <map>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "VulkanUtil.hpp"
#include "World.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const float ANIMATION_TIME = 2.0f;

typedef std::array<int, 6> SideIndex;

float dot(vec5 a, vec5 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.u * b.u + a.v * b.v;
}

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

class App {
   public:
    void run() {
        initWindow();
        initVulkan();
        createSurface();
        initWorld();
        mainLoop();
        cleanup();
    }

   private:
    GLFWwindow *window;
    VulkanUtil vulkan;
    World world = World(&vulkan);
    std::vector<int> cells;
    std::map<SideIndex, size_t> sideIndices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices;
    long vertexIndex = 0;
    long indexIndex = 0;

    int buildMat = 1;

    bool cursorLocked = false;
    bool firstMousePosition = true;

    bool framebufferResized = false;

    double lastX = 0;
    double lastY = 0;

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

    void initWorld() {
        world.init();
        int size = 1;
        for (int x = -size; x <= size; x += 1) {
            for (int z = -size; z <= size; z += 1) {
                for (int u = -size; u <= size; u += 1) {
                    for (int v = -size; v <= size; v += 1) {
                        std::cerr << x << "," << z << "," << u << "," << v << std::endl;
                        world.loadChunk({x, 0, z, u, v});
                        world.saveChunk({x, 0, z, u, v});
                    }
                }
            }
        }
        world.printStats();
        world.sendVerticesAndIndicesToVulkan();
    }

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
        app->vulkan.framebufferResized = true;
    }

    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
        auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            if (!app->cursorLocked) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                app->cursorLocked = true;
                app->firstMousePosition = true;
            } else {
                app->world.setCell(app->focusedCell, 0);
                app->world.sendVerticesAndIndicesToVulkan();
                app->world.saveChunk(app->world.chunkLocForCell(app->focusedCell));
            }
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            if (app->cursorLocked) {
                app->world.setCell(app->buildCell, app->buildMat);
                app->world.sendVerticesAndIndicesToVulkan();
                app->world.saveChunk(app->world.chunkLocForCell(app->buildCell));
            }
        }
    }

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
            } else if (key == GLFW_KEY_V) {
                app->uvViewTarget = 1.0 - app->uvViewTarget;
            } else if (key == GLFW_KEY_SPACE) {
                if (!app->flying) {
                    app->holdingJump = true;
                } else {
                    app->upVel = app->walkVel;
                }
            } else if (key == GLFW_KEY_LEFT_SHIFT) {
                app->uvTravel = true;
                app->oldUv = app->uv;
            } else if (key == GLFW_KEY_ESCAPE) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                app->cursorLocked = false;
            } else if (key == GLFW_KEY_1) {
                app->buildMat = 1;
            } else if (key == GLFW_KEY_2) {
                app->buildMat = 2;
            } else if (key == GLFW_KEY_3) {
                app->buildMat = 3;
            } else if (key == GLFW_KEY_G) {
                app->flying = !app->flying;
            } else if (key == GLFW_KEY_Z) {
                if (app->flying) {
                    app->downVel = app->walkVel;
                }
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
            } else if (key == GLFW_KEY_Z) {
                app->downVel = 0.0f;
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
    bool flying = false;
    glm::vec3 loc = glm::vec3(CHUNK_SIZE_XZUV / 2 + 0.5f, CHUNK_SIZE_Y / 2 + 3, CHUNK_SIZE_XZUV / 2 + 0.5f);
    glm::vec2 uv = glm::vec2(CHUNK_SIZE_XZUV / 2 + 0.5f, CHUNK_SIZE_XZUV / 2 + 0.5f);
    glm::vec3 lookHeading = glm::vec3(1.0f, 0.0f, 0.0f);
    float lookAltitude = 0.0f;
    float height = 1.75f;
    float radius = 0.25f;
    glm::vec2 oldUv = glm::vec2(CHUNK_SIZE_XZUV / 2, CHUNK_SIZE_XZUV / 2);
    bool holdingJump = false;
    bool inJump = false;
    CellLoc focusedCell;
    CellLoc buildCell;
    float uvView = 0.0f;
    float uvViewTarget = 0.0f;

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
        vec5 pos = {loc.x, loc.y - (height - h), loc.z, uv.x, uv.y};
        vec5 d = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(u), static_cast<float>(v)};
        vec5 cell = floor(pos) + 0.5f;
        vec5 adjCell = cell + d;
        int material = world.getCell({adjCell});
        if (material != 0) {
            if (y != 0) {
                // TODO
            } else {
                vec5 nLoc = cell + d / 2.0f;
                vec5 aLoc = cell + d;
                vec5 cNorm = normalize(nLoc - aLoc);
                cNorm = normalize(cNorm - project(cNorm, {0, 1, 0, 0, 0}));
                float dist2Plane = dot(cNorm, pos - nLoc);
                if (dist2Plane < radius) {
                    float move = radius - dist2Plane;
                    cNorm = cNorm * move;
                    loc = loc + cNorm.xyz();
                    uv = uv + cNorm.uv();
                }
            }
        }
    }

    void updatePosition(float time) {
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::cross(lookHeading, up);
        glm::vec3 feet = loc - (up * height);
        int feetCell = world.getCell({feet, uv});
        bool falling = feetCell == 0 && !flying;
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
        if (flying) {
            playerVel = up * (upVel - downVel);
        }
        playerVel = playerVel + lookHeading * (forwardVel - backVel);
        playerVel = playerVel + right * (rightVel - leftVel);

        // Move
        if (uvTravel == (uvView < 0.5f)) {
            uv = uv + (glm::vec2(playerVel.x, playerVel.z) * time);
            loc.y = loc.y + (playerVel.y * time);
        } else {
            loc = loc + (playerVel * time);
        }

        // Snap hidden dimensions
        if (!uvTravel) {
            if (uvView < 0.5f) {
                glm::vec2 rounded = glm::round(uv - 0.5f) + 0.5f;
                if (glm::length(uv - rounded) < 0.01f) {
                    uv = rounded;
                } else {
                    float alpha = glm::exp(-time * 7.0f / ANIMATION_TIME);
                    uv = (1.0f - alpha) * rounded + alpha * uv;
                }
            } else {
                glm::vec2 hide = glm::vec2(loc.x, loc.z);
                glm::vec2 rounded = glm::round(hide - 0.5f) + 0.5f;
                if (glm::length(hide - rounded) < 0.01f) {
                    loc.x = rounded.x;
                    loc.z = rounded.y;
                } else {
                    float alpha = glm::exp(-time * 7.0f / ANIMATION_TIME);
                    loc.x = (1.0f - alpha) * rounded.x + alpha * hide.x;
                    loc.z = (1.0f - alpha) * rounded.y + alpha * hide.y;
                }
            }
        }

        if (!flying) {
            // if (collide(2)) {
            //     fallVel = 0.0f;
            // }
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
        }

        glm::vec3 look = lookDir();
        vec5 increment = {0.0, look.y, 0.0, 0.0, 0.0};
        if (uvView < 0.5f) {
            increment.x = look.x;
            increment.z = look.z;
        } else {
            increment.u = look.x;
            increment.v = look.z;
        }
        increment = increment * 0.05f;
        vec5 pos = {loc.x, loc.y, loc.z, uv.x, uv.y};
        focusedCell = {};
        CellLoc prevCell;
        for (int i = 0; i < 100; i++) {
            pos = pos + increment;
            CellLoc cellLoc = {pos};
            int cell = world.getCell(cellLoc);
            if (cell != 0) {
                focusedCell = cellLoc;
                buildCell = prevCell;
                break;
            }
            prevCell = {pos};
	    }
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
        glm::vec3 eye = loc;
        if (uvView > 0.5f) {
            eye = glm::vec3(uv.x, loc.y, uv.y);
        }
        ubo.view = glm::lookAt(eye, eye + look, glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), vulkan.swapChainExtent.width / (float)vulkan.swapChainExtent.height, 0.02f, 50.0f);
        ubo.proj[1][1] *= -1;
        ubo.xyz = loc;
        ubo.uv = uv;

        // ubo.selectedCell = focusedCell;
        // ubo.selectedCellUV = focusedCellUV;

        if (glm::abs(uvViewTarget - uvView) < 0.01f) {
            uvView = uvViewTarget;
        } else {
            float alpha = glm::exp(-timeDelta * 7.0f / ANIMATION_TIME);
            uvView = (1.0f - alpha) * uvViewTarget + alpha * uvView;
        }
        ubo.uvView = uvView;

        vulkan.ubo = ubo;
    }
};

int main() {
    App app;

    // try {
    app.run();
    // } catch (const std::exception &e) {
    //     std::cerr << e.what() << std::endl;
    //     return EXIT_FAILURE;
    // }

    return EXIT_SUCCESS;
}
