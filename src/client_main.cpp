#include <iostream>
#include <memory>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "engine/core/glm_util.hpp"
#include "engine/core/FlatChunkGenerator.hpp"
#include "engine/core/LocalChunkHandler.hpp"
#include "engine/core/World.hpp"
// #include "engine/network/WorldClient.hpp"
#include "engine/render/WorldView.hpp"

std::vector<const char *> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}

class WindowTask : public WorldTask {
public:

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto app = reinterpret_cast<WindowTask *>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
        auto app = reinterpret_cast<WindowTask *>(glfwGetWindowUserPointer(window));
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            if (!app->cursorLocked) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                app->cursorLocked = true;
                app->firstMousePosition = true;
            } else {
                app->removeCell = true;
            }
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            if (app->cursorLocked) {
                app->setCell = true;
            }
        }
    }

    static void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos) {
        auto app = reinterpret_cast<WindowTask *>(glfwGetWindowUserPointer(window));
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
        auto app = reinterpret_cast<WindowTask *>(glfwGetWindowUserPointer(window));
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

    GLFWwindow *window;
    WorldView &view;
    float forwardVel = 0.0f;
    float backVel = 0.0f;
    float leftVel = 0.0f;
    float rightVel = 0.0f;
    float upVel = 0.0f;
    float downVel = 0.0f;
    float fallVel = 0.0f;
    float walkVel = 2.0f;
    float uvViewTarget = 0.0f;
    int buildMat = 1;
    bool flying = false;
    bool holdingJump = false;
    bool inJump = false;
    bool uvTravel = false;
    bool cursorLocked = false;
    bool firstMousePosition = true;
    double lastX;
    double lastY;
    CellLoc focusedCell;
    CellLoc buildCell;
    bool framebufferResized = false;
    glm::vec3 lookHeading = glm::vec3(1.0f, 0.0f, 0.0f);
    float lookAltitude = 0.0f;
    bool setCell = false;
    bool removeCell = false;
    float height = 1.75f;
    float radius = 0.25f;
    vec5 location = {2, 5, 2, 2.5, 2.5};

    WindowTask(GLFWwindow *appWindow, WorldView &worldView) : window(appWindow), view(worldView) {
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPositionCallback);
    }

    void executeTask(World &world, float timeDelta) override {
        glfwPollEvents();

        if (removeCell) {
            world.setCell(focusedCell, 0);
            removeCell = false;
        }
        if (setCell) {
            world.setCell(buildCell, buildMat);
            setCell = false;
        }

        updateLocation(world, timeDelta);
        updateFocusedCell(world);
        view.setCameraUVView(uvViewTarget);

        if (glfwWindowShouldClose(window)) {
            world.stop();
        }
    }

    WorldPos worldPosFromVec5(vec5 v) {
        return {
            v.x,
            v.y,
            v.z,
            v.u,
            v.v,
        };
    }

    CellLoc cellLocFromVec5(vec5 v) {
        vec5 floorPos = floor(v);
        return {
            static_cast<int>(floorPos.x),
            static_cast<int>(floorPos.y),
            static_cast<int>(floorPos.z),
            static_cast<int>(floorPos.u),
            static_cast<int>(floorPos.v),
        };
    }

    glm::vec3 lookDir() {
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        lookHeading = glm::normalize(projectToPlane(lookHeading, up));
        glm::vec3 right = glm::cross(lookHeading, up);
        return glm::rotate(glm::mat4(1.0f), glm::radians(lookAltitude - 90.0f), right) * glm::vec4(up, 1.0f);
    }

    void updateLocation(World &world, float time) {
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::cross(lookHeading, up);
        vec5 feet = location;
        feet.y -= height;
        int feetCell = world.getCell(cellLocFromVec5(feet));
        bool falling = feetCell == 0 && !flying;
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
        if (uvTravel == (view.getCameraUVView() < 0.5f)) {
            location.u += playerVel.x * time;
            location.v += playerVel.z * time;
            location.y += playerVel.y * time;
        } else {
            location.x += playerVel.x * time;
            location.z += playerVel.z * time;
            location.y += playerVel.y * time;
        }

        if (!flying) {
            for (float h = 0.5f; h < height; h += 1.0f) {
                collide(world, h, 1, 0, 0, 0, 0);
                collide(world, h, -1, 0, 0, 0, 0);
                collide(world, h, 0, 0, 1, 0, 0);
                collide(world, h, 0, 0, -1, 0, 0);
                collide(world, h, 0, 0, 0, 1, 0);
                collide(world, h, 0, 0, 0, -1, 0);
                collide(world, h, 0, 0, 0, 0, 1);
                collide(world, h, 0, 0, 0, 0, -1);
            }
        }

        view.setCameraPosition(worldPosFromVec5(location));
        // std::cout << "loc:  " << location.x << "," << location.y << "," << location.z << "," << location.u << "," << location.v << std::endl;

        glm::vec3 look = lookDir();
        vec5 lookAt = location;
        lookAt.y += look.y;
        if (view.getCameraUVView() < 0.5f) {
            lookAt.x += look.x;
            lookAt.z += look.z;
        } else {
            lookAt.u += look.x;
            lookAt.v += look.z;
        }
        view.setCameraLookAt(worldPosFromVec5(lookAt));
        // std::cout << "look: " << lookAt.x << "," << lookAt.y << "," << lookAt.z << "," << lookAt.u << "," << lookAt.v << std::endl;
    }

    void collide(World &world, float h, int x, int y, int z, int u, int v) {
        vec5 pos = location;
        pos.y -= height - h;
        vec5 d = {
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(z),
            static_cast<float>(u),
            static_cast<float>(v),
        };
        vec5 cell = floor(pos) + 0.5f;
        vec5 adjCell = cell + d;
        int material = world.getCell(cellLocFromVec5(adjCell));
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
                    location = location + cNorm * move;
                }
            }
        }
    }

    void updateFocusedCell(World &world) {
        glm::vec3 look = lookDir();
        vec5 increment = {0.0, look.y, 0.0, 0.0, 0.0};
        if (view.getCameraUVView() < 0.5f) {
            increment.x = look.x;
            increment.z = look.z;
        } else {
            increment.u = look.x;
            increment.v = look.z;
        }
        increment = increment * 0.05f;
        vec5 pos = location;
        focusedCell = {};
        CellLoc prevCell;
        for (int i = 0; i < 100; i++) {
            pos = pos + increment;
            CellLoc cellLoc = cellLocFromVec5(pos);
            int cell = world.getCell(cellLoc);
            if (cell != 0) {
                focusedCell = cellLoc;
                buildCell = prevCell;
                break;
            }
            prevCell = cellLoc;
        }
    }

    void swivel(float deltaX, float deltaY) {
        float lookHeadingDelta = -0.1f * deltaX;
        glm::vec3 normalDir(0.0f, 1.0f, 0.0f);
        lookHeading = glm::rotate(glm::mat4(1.0f), glm::radians(lookHeadingDelta), normalDir) * glm::vec4(lookHeading, 1.0f);
        lookAltitude = lookAltitude - 0.1f * deltaY;
        lookAltitude = std::max(std::min(lookAltitude, 89.9f), -89.9f);
    }
};

int main(int argc, char *argv[]) {
    World world;
    if (argc == 3 && std::string(argv[1]) == "--server") {
        std::cout << "Not implemented" << std::endl;
        // auto client = std::make_shared<WorldClient>(argv[2]);
        // world.addWorldListener(client);
        // world.addWorldTask(client);
        // world.setChunkRequestHandler(client);
        // world.run();
    } else if (argc == 2) {
        glfwInit();
        auto view = std::make_shared<WorldView>(getRequiredExtensions());
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        auto window = glfwCreateWindow(500, 500, "5d", nullptr, nullptr);
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(view->getInstance(), window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        view->initSurface(surface);
        world.addWorldListener(view);
        world.addWorldTask(view);

        auto gen = std::make_unique<FlatChunkGenerator>();
        auto handler = std::make_shared<LocalChunkHandler>(argv[1], std::move(gen));
        world.addWorldListener(handler);
        world.setChunkRequestHandler(handler);

        auto windowTask = std::make_shared<WindowTask>(window, *view);
        world.addWorldTask(windowTask);

        world.run();
    } else {
        std::cout << "Usage:" << std::endl;
        std::cout << "  5d --server [server:port]" << std::endl;
        std::cout << "  5d [world_dir]" << std::endl;
    }
    return 0;
}
