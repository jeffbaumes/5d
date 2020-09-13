#include <iostream>
#include <memory>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
    WindowTask(GLFWwindow *appWindow) : window(appWindow) { }

    void executeTask(World &world, float dt) override {
        glfwPollEvents();
        if (glfwWindowShouldClose(window)) {
            world.stop();
        }
    }

    GLFWwindow *window;
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
        // glfwSetWindowUserPointer(window, this);
        // glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        // glfwSetKeyCallback(window, keyCallback);
        // glfwSetMouseButtonCallback(window, mouseButtonCallback);
        // glfwSetCursorPosCallback(window, cursorPositionCallback);
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

        auto windowTask = std::make_shared<WindowTask>(window);
        world.addWorldTask(windowTask);

        world.run();
    } else {
        std::cout << "Usage:" << std::endl;
        std::cout << "  5d --server [server:port]" << std::endl;
        std::cout << "  5d [world_dir]" << std::endl;
    }
    return 0;
}
