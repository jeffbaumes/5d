#include "App.hpp"

#include "Entity.hpp"
#include "WorldClient.hpp"

App::App() {
    world = new World(&vulkan);
}

App::App(WorldClient *client) {
    world = new World(&vulkan, client);
}

App::~App() {
    delete world;
    world = nullptr;
}

void App::run() {
    initWindow();
    initVulkan();
    createSurface();
    initWorld();
    mainLoop();
    cleanup();
}

void App::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIDTH, HEIGHT, "5d", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
}

void App::initVulkan() {
    vulkan.init();
}

void App::initWorld() {
    world->init();
    // world->loadChunk({0, 0, 0, 0, 0});
    auto entity = Entity(world, 3);
    entity.init();
    world->entities.push_back(entity);
    int size = 1;
    for (int x = -size; x <= size; x += 1) {
        for (int z = -size; z <= size; z += 1) {
            for (int u = -size; u <= size; u += 1) {
                for (int v = -size; v <= size; v += 1) {
                    std::cerr << x << "," << z << "," << u << "," << v << std::endl;
                    world->loadChunk({x, 0, z, u, v});
                }
            }
        }
    }
    world->printStats();
    world->sendVerticesAndIndicesToVulkan();
}

void App::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
    app->vulkan.framebufferResized = true;
}

void App::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (!app->cursorLocked) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            app->cursorLocked = true;
            app->firstMousePosition = true;
        } else {
            app->world->setCell(app->focusedCell, 0);
            app->world->sendVerticesAndIndicesToVulkan();
            app->world->saveChunk(app->world->chunkLocForCell(app->focusedCell));
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        if (app->cursorLocked) {
            app->world->setCell(app->buildCell, app->buildMat);
            app->world->sendVerticesAndIndicesToVulkan();
            app->world->saveChunk(app->world->chunkLocForCell(app->buildCell));
        }
    }
}

void App::cursorPositionCallback(GLFWwindow *window, double xpos, double ypos) {
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

void App::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
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

void App::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        updateUniforms();
        vulkan.draw();
        world->pollEvents();
    }
}

void App::cleanup() {
    vulkan.cleanup();

    glfwDestroyWindow(window);

    glfwTerminate();
}

void App::createSurface() {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(vulkan.instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
    vulkan.initSurface(surface);
}

glm::vec3 App::lookDir() {
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    lookHeading = glm::normalize(projectToPlane(lookHeading, up));
    glm::vec3 right = glm::cross(lookHeading, up);
    return glm::rotate(glm::mat4(1.0f), glm::radians(lookAltitude - 90.0f), right) * glm::vec4(up, 1.0f);
}

void App::swivel(float deltaX, float deltaY) {
    float lookHeadingDelta = -0.1f * deltaX;
    glm::vec3 normalDir(0.0f, 1.0f, 0.0f);
    lookHeading = glm::rotate(glm::mat4(1.0f), glm::radians(lookHeadingDelta), normalDir) * glm::vec4(lookHeading, 1.0f);
    lookAltitude = lookAltitude - 0.1f * deltaY;
    lookAltitude = std::max(std::min(lookAltitude, 89.9f), -89.9f);
}

void App::collide(float h, int x, int y, int z, int u, int v) {
    vec5 pos = location;
    pos.y -= height - h;
    vec5 d(x, y, z, u, v);
    vec5 cell = floor(pos) + 0.5f;
    vec5 adjCell = cell + d;
    int material = world->getCell(floor(adjCell));
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

void App::updatePosition(float time) {
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::cross(lookHeading, up);
    vec5 feet = location;
    feet.y -= height;
    int feetCell = world->getCell(floor(feet));
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
    if (uvTravel == (uvView < 0.5f)) {
        location.u += playerVel.x * time;
        location.v += playerVel.z * time;
        location.y += playerVel.y * time;
    } else {
        location.x += playerVel.x * time;
        location.z += playerVel.z * time;
        location.y += playerVel.y * time;
    }

    // Snap hidden dimensions
    if (!uvTravel) {
        if (uvView < 0.5f) {
            glm::vec2 hide = location.uv();
            glm::vec2 rounded = glm::round(hide - 0.5f) + 0.5f;
            if (glm::length(hide - rounded) < 0.01f) {
                location.u = rounded.x;
                location.v = rounded.y;
            } else {
                float alpha = glm::exp(-time * 7.0f / ANIMATION_TIME);
                location.u = (1.0f - alpha) * rounded.x + alpha * location.u;
                location.v = (1.0f - alpha) * rounded.y + alpha * location.v;
            }
        } else {
            glm::vec2 hide = glm::vec2(location.x, location.z);
            glm::vec2 rounded = glm::round(hide - 0.5f) + 0.5f;
            if (glm::length(hide - rounded) < 0.01f) {
                location.x = rounded.x;
                location.z = rounded.y;
            } else {
                float alpha = glm::exp(-time * 7.0f / ANIMATION_TIME);
                location.x = (1.0f - alpha) * rounded.x + alpha * hide.x;
                location.z = (1.0f - alpha) * rounded.y + alpha * hide.y;
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
    vec5 pos = location;
    focusedCell = {};
    CellLoc prevCell;
    for (int i = 0; i < 100; i++) {
        pos = pos + increment;
        CellLoc cellLoc = floor(pos);
        int cell = world->getCell(cellLoc);
        if (cell != 0) {
            focusedCell = cellLoc;
            buildCell = prevCell;
            break;
        }
        prevCell = cellLoc;
    }
}

void App::updateUniforms() {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    if (firstTime) {
        lastTime = currentTime;
        firstTime = false;
    }
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
    lastTime = currentTime;

    // world->entities[0].location = {glm::sin(time) * 2 + 2, 2, glm::sin(time) * 2 + 2, 0, 0};
    // world->entities[0].location = {glm::sin(time), 2, 2, floor(uv.x), floor(uv.y)};
    world->entities[0].location = {2 + glm::sin(time), 2, 2, 2, 2};
    world->entities[0].rotation = glm::vec3(glm::sin(time), glm::sin(time), 0);
    // std::cout << world->entities[0].location.u << "," << world->entities[0].location.v << "," << world->entities[0].location.x << "," << std::endl;
    // std::cout << loc.x << "," << loc.y << "," << loc.z << "," << uv.x << "," << uv.y << std::endl;

    updatePosition(timeDelta);

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), 0.0f * time * glm::radians(90.0f) / 4.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec3 look = lookDir();
    glm::vec3 eye = location.xyz();
    if (uvView > 0.5f) {
        eye = glm::vec3(location.u, location.y, location.v);
    }
    ubo.view = glm::lookAt(eye, eye + look, glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), vulkan.swapChainExtent.width / (float)vulkan.swapChainExtent.height, 0.02f, 50.0f);
    ubo.proj[1][1] *= -1;
    ubo.xyz = location.xyz();
    ubo.uv = location.uv();

    // ubo.selectedCell = focusedCell;
    // ubo.selectedCellUV = focusedCellUV;

    if (glm::abs(uvViewTarget - uvView) < 0.01f) {
        uvView = uvViewTarget;
    } else {
        float alpha = glm::exp(-timeDelta * 7.0f / ANIMATION_TIME);
        uvView = (1.0f - alpha) * uvViewTarget + alpha * uvView;
    }
    ubo.uvView = uvView;

    // std::cerr << "before" << std::endl << std::flush;

    world->updateUBO(&ubo);

    // std::cerr << "after" << std::endl << std::flush;

    // std::cout << ubo.entityLocationUV[world->entities[0].id()].x << "," << ubo.entityLocationUV[world->entities[0].id()].y << "," << ubo.entityLocationXYZ[world->entities[0].id()].z << "," << std::endl;

    vulkan.ubo = ubo;
}
