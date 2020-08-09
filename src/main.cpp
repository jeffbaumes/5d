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
#include "Entity.hpp"

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
    // std::unordered_map<Vertex, uint32_t> uniqueVertices;
    // long vertexIndex = 0;
    // long indexIndex = 0;

    int buildMat = 1;

    bool cursorLocked = false;
    bool firstMousePosition = true;

    bool framebufferResized = false;

    double lastX = 0;
    double lastY = 0;

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "5d", nullptr, nullptr);
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
        // world.loadChunk({0, 0, 0, 0, 0});
        auto entity = Entity(&world, 3);
        entity.init();
        world.entities.push_back(entity);
        int size = 1;
        for (int x = -size; x <= size; x += 1) {
            for (int z = -size; z <= size; z += 1) {
                for (int u = -size; u <= size; u += 1) {
                    for (int v = -size; v <= size; v += 1) {
                        std::cerr << x << "," << z << "," << u << "," << v << std::endl;
                        world.loadChunk({x, 0, z, u, v});
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
        vec5 pos = location;
        pos.y -= height - h;
        vec5 d(x, y, z, u, v);
        vec5 cell = floor(pos) + 0.5f;
        vec5 adjCell = cell + d;
        int material = world.getCell(floor(adjCell));
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

    void updatePosition(float time) {
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::cross(lookHeading, up);
        vec5 feet = location;
        feet.y -= height;
        int feetCell = world.getCell(floor(feet));
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
            int cell = world.getCell(cellLoc);
            if (cell != 0) {
                focusedCell = cellLoc;
                buildCell = prevCell;
                break;
            }
            prevCell = cellLoc;
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

        // world.entities[0].location = {glm::sin(time) * 2 + 2, 2, glm::sin(time) * 2 + 2, 0, 0};
        // world.entities[0].location = {glm::sin(time), 2, 2, floor(uv.x), floor(uv.y)};
        world.entities[0].location = {2 + glm::sin(time), 2, 2, 2, 2};
        world.entities[0].rotation = glm::vec3(glm::sin(time), glm::sin(time), 0);
        // std::cout << world.entities[0].location.u << "," << world.entities[0].location.v << "," << world.entities[0].location.x << "," << std::endl;
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

        world.updateUBO(&ubo);

        // std::cerr << "after" << std::endl << std::flush;

        // std::cout << ubo.entityLocationUV[world.entities[0].id()].x << "," << ubo.entityLocationUV[world.entities[0].id()].y << "," << ubo.entityLocationXYZ[world.entities[0].id()].z << "," << std::endl;

        vulkan.ubo = ubo;
    }
};

// int main() {
//     App app;
//     // try {
//     app.run();
//     // } catch (const std::exception &e) {
//     //     std::cerr << e.what() << std::endl;
//     //     return EXIT_FAILURE;
//     // }
//     return EXIT_SUCCESS;
// }

//====== Copyright Valve Corporation, All rights reserved. ====================
//
// Example client/server chat application using SteamNetworkingSockets

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#include <string.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <map>
#include <mutex>
#include <queue>
#include <random>
#include <string>
#include <thread>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
// #include <steam/steam_api.h>
#endif

#ifdef WIN32
#include <windows.h>  // Ug, for NukeProcess -- see below
#else
#include <signal.h>
#include <unistd.h>
#endif

/////////////////////////////////////////////////////////////////////////////
//
// Common stuff
//
/////////////////////////////////////////////////////////////////////////////

bool g_bQuit = false;

SteamNetworkingMicroseconds g_logTimeZero;

// We do this because I won't want to figure out how to cleanly shut
// down the thread that is reading from stdin.
static void NukeProcess(int rc) {
#ifdef WIN32
    ExitProcess(rc);
#else
    kill(getpid(), SIGKILL);
#endif
}

static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg) {
    SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
    printf("%10.6f %s\n", time * 1e-6, pszMsg);
    fflush(stdout);
    if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
        fflush(stdout);
        fflush(stderr);
        NukeProcess(1);
    }
}

static void FatalError(const char *fmt, ...) {
    char text[2048];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);
    char *nl = strchr(text, '\0') - 1;
    if (nl >= text && *nl == '\n')
        *nl = '\0';
    DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Bug, text);
}

static void Printf(const char *fmt, ...) {
    char text[2048];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);
    char *nl = strchr(text, '\0') - 1;
    if (nl >= text && *nl == '\n')
        *nl = '\0';
    DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Msg, text);
}

static void InitSteamDatagramConnectionSockets() {
#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
    SteamDatagramErrMsg errMsg;
    if (!GameNetworkingSockets_Init(nullptr, errMsg))
        FatalError("GameNetworkingSockets_Init failed.  %s", errMsg);
#else
    // SteamDatagramClient_SetAppID(570);  // Just set something, doesn't matter what
    // //SteamDatagramClient_SetUniverse( k_EUniverseDev );

    // SteamDatagramErrMsg errMsg;
    // if (!SteamDatagramClient_Init(true, errMsg))
    //     FatalError("SteamDatagramClient_Init failed.  %s", errMsg);

    // // Disable authentication when running with Steam, for this
    // // example, since we're not a real app.
    // //
    // // Authentication is disabled automatically in the open-source
    // // version since we don't have a trusted third party to issue
    // // certs.
    // SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
#endif

    g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

    SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
}

static void ShutdownSteamDatagramConnectionSockets() {
    // Give connections time to finish up.  This is an application layer protocol
    // here, it's not TCP.  Note that if you have an application and you need to be
    // more sure about cleanup, you won't be able to do this.  You will need to send
    // a message and then either wait for the peer to close the connection, or
    // you can pool the connection to see if any reliable data is pending.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
    GameNetworkingSockets_Kill();
#else
    // SteamDatagramClient_Kill();
#endif
}

/////////////////////////////////////////////////////////////////////////////
//
// Non-blocking console user input.  Sort of.
// Why is this so hard?
//
/////////////////////////////////////////////////////////////////////////////

std::mutex mutexUserInputQueue;
std::queue<std::string> queueUserInput;

std::thread *s_pThreadUserInput = nullptr;
std::thread *s_pThreadMain = nullptr;

void LocalUserInput_Init() {
    s_pThreadUserInput = new std::thread([]() {
        while (!g_bQuit) {
            char szLine[4000];
            if (!fgets(szLine, sizeof(szLine), stdin)) {
                // Well, you would hope that you could close the handle
                // from the other thread to trigger this.  Nope.
                if (g_bQuit)
                    return;
                g_bQuit = true;
                Printf("Failed to read on stdin, quitting\n");
                break;
            }

            mutexUserInputQueue.lock();
            queueUserInput.push(std::string(szLine));
            mutexUserInputQueue.unlock();
        }
    });
}

void LocalUserInput_Kill() {
    // Does not work.  We won't clean up, we'll just nuke the process.
    //	g_bQuit = true;
    //	_close( fileno( stdin ) );
    //
    //	if ( s_pThreadUserInput )
    //	{
    //		s_pThreadUserInput->join();
    //		delete s_pThreadUserInput;
    //		s_pThreadUserInput = nullptr;
    //	}
}

// You really gotta wonder what kind of pedantic garbage was
// going through the minds of people who designed std::string
// that they decided not to include trim.
// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                return !std::isspace(ch);
            }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
                return !std::isspace(ch);
            }).base(),
            s.end());
}

// Read the next line of input from stdin, if anything is available.
bool LocalUserInput_GetNext(std::string &result) {
    bool got_input = false;
    mutexUserInputQueue.lock();
    while (!queueUserInput.empty() && !got_input) {
        result = queueUserInput.front();
        queueUserInput.pop();
        ltrim(result);
        rtrim(result);
        got_input = !result.empty();  // ignore blank lines
    }
    mutexUserInputQueue.unlock();
    return got_input;
}

/////////////////////////////////////////////////////////////////////////////
//
// ChatServer
//
/////////////////////////////////////////////////////////////////////////////

class ChatServer {
   public:
    void Run(uint16 nPort) {
        // Select instance to use.  For now we'll always use the default.
        // But we could use SteamGameServerNetworkingSockets() on Steam.
        m_pInterface = SteamNetworkingSockets();

        // Start listening
        SteamNetworkingIPAddr serverLocalAddr;
        serverLocalAddr.Clear();
        serverLocalAddr.m_port = nPort;
        SteamNetworkingConfigValue_t opt;
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void *)SteamNetConnectionStatusChangedCallback);
        m_hListenSock = m_pInterface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
        if (m_hListenSock == k_HSteamListenSocket_Invalid)
            FatalError("Failed to listen on port %d", nPort);
        m_hPollGroup = m_pInterface->CreatePollGroup();
        if (m_hPollGroup == k_HSteamNetPollGroup_Invalid)
            FatalError("Failed to listen on port %d", nPort);
        Printf("Server listening on port %d\n", nPort);

        while (!g_bQuit) {
            PollIncomingMessages();
            PollConnectionStateChanges();
            PollLocalUserInput();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // Close all the connections
        Printf("Closing connections...\n");
        for (auto it : m_mapClients) {
            // Send them one more goodbye message.  Note that we also have the
            // connection close reason as a place to send final data.  However,
            // that's usually best left for more diagnostic/debug text not actual
            // protocol strings.
            SendStringToClient(it.first, "Server is shutting down.  Goodbye.");

            // Close the connection.  We use "linger mode" to ask SteamNetworkingSockets
            // to flush this out and close gracefully.
            m_pInterface->CloseConnection(it.first, 0, "Server Shutdown", true);
        }
        m_mapClients.clear();

        m_pInterface->CloseListenSocket(m_hListenSock);
        m_hListenSock = k_HSteamListenSocket_Invalid;

        m_pInterface->DestroyPollGroup(m_hPollGroup);
        m_hPollGroup = k_HSteamNetPollGroup_Invalid;
    }

   private:
    HSteamListenSocket m_hListenSock;
    HSteamNetPollGroup m_hPollGroup;
    ISteamNetworkingSockets *m_pInterface;

    struct Client_t {
        std::string m_sNick;
    };

    std::map<HSteamNetConnection, Client_t> m_mapClients;

    void SendStringToClient(HSteamNetConnection conn, const char *str) {
        m_pInterface->SendMessageToConnection(conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr);
    }

    void SendStringToAllClients(const char *str, HSteamNetConnection except = k_HSteamNetConnection_Invalid) {
        for (auto &c : m_mapClients) {
            if (c.first != except)
                SendStringToClient(c.first, str);
        }
    }

    void PollIncomingMessages() {
        char temp[1024];

        while (!g_bQuit) {
            ISteamNetworkingMessage *pIncomingMsg = nullptr;
            int numMsgs = m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, &pIncomingMsg, 1);
            if (numMsgs == 0)
                break;
            if (numMsgs < 0)
                FatalError("Error checking for messages");
            assert(numMsgs == 1 && pIncomingMsg);
            auto itClient = m_mapClients.find(pIncomingMsg->m_conn);
            assert(itClient != m_mapClients.end());

            // '\0'-terminate it to make it easier to parse
            std::string sCmd;
            sCmd.assign((const char *)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
            const char *cmd = sCmd.c_str();

            // We don't need this anymore.
            pIncomingMsg->Release();

            // Check for known commands.  None of this example code is secure or robust.
            // Don't write a real server like this, please.

            if (strncmp(cmd, "/nick", 5) == 0) {
                const char *nick = cmd + 5;
                while (isspace(*nick))
                    ++nick;

                // Let everybody else know they changed their name
                sprintf(temp, "%s shall henceforth be known as %s", itClient->second.m_sNick.c_str(), nick);
                SendStringToAllClients(temp, itClient->first);

                // Respond to client
                sprintf(temp, "Ye shall henceforth be known as %s", nick);
                SendStringToClient(itClient->first, temp);

                // Actually change their name
                SetClientNick(itClient->first, nick);
                continue;
            }

            // Assume it's just a ordinary chat message, dispatch to everybody else
            sprintf(temp, "%s: %s", itClient->second.m_sNick.c_str(), cmd);
            SendStringToAllClients(temp, itClient->first);
        }
    }

    void PollLocalUserInput() {
        std::string cmd;
        while (!g_bQuit && LocalUserInput_GetNext(cmd)) {
            if (strcmp(cmd.c_str(), "/quit") == 0) {
                g_bQuit = true;
                Printf("Shutting down server");
                break;
            }

            // That's the only command we support
            Printf("The server only knows one command: '/quit'");
        }
    }

    void SetClientNick(HSteamNetConnection hConn, const char *nick) {
        // Remember their nick
        m_mapClients[hConn].m_sNick = nick;

        // Set the connection name, too, which is useful for debugging
        m_pInterface->SetConnectionName(hConn, nick);
    }

    void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo) {
        char temp[1024];

        // What's the state of the connection?
        switch (pInfo->m_info.m_eState) {
            case k_ESteamNetworkingConnectionState_None:
                // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
                break;

            case k_ESteamNetworkingConnectionState_ClosedByPeer:
            case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
                // Ignore if they were not previously connected.  (If they disconnected
                // before we accepted the connection.)
                if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected) {
                    // Locate the client.  Note that it should have been found, because this
                    // is the only codepath where we remove clients (except on shutdown),
                    // and connection change callbacks are dispatched in queue order.
                    auto itClient = m_mapClients.find(pInfo->m_hConn);
                    assert(itClient != m_mapClients.end());

                    // Select appropriate log messages
                    const char *pszDebugLogAction;
                    if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
                        pszDebugLogAction = "problem detected locally";
                        sprintf(temp, "Alas, %s hath fallen into shadow.  (%s)", itClient->second.m_sNick.c_str(), pInfo->m_info.m_szEndDebug);
                    } else {
                        // Note that here we could check the reason code to see if
                        // it was a "usual" connection or an "unusual" one.
                        pszDebugLogAction = "closed by peer";
                        sprintf(temp, "%s hath departed", itClient->second.m_sNick.c_str());
                    }

                    // Spew something to our own log.  Note that because we put their nick
                    // as the connection description, it will show up, along with their
                    // transport-specific data (e.g. their IP address)
                    Printf("Connection %s %s, reason %d: %s\n",
                           pInfo->m_info.m_szConnectionDescription,
                           pszDebugLogAction,
                           pInfo->m_info.m_eEndReason,
                           pInfo->m_info.m_szEndDebug);

                    m_mapClients.erase(itClient);

                    // Send a message so everybody else knows what happened
                    SendStringToAllClients(temp);
                } else {
                    assert(pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
                }

                // Clean up the connection.  This is important!
                // The connection is "closed" in the network sense, but
                // it has not been destroyed.  We must close it on our end, too
                // to finish up.  The reason information do not matter in this case,
                // and we cannot linger because it's already closed on the other end,
                // so we just pass 0's.
                m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
                break;
            }

            case k_ESteamNetworkingConnectionState_Connecting: {
                // This must be a new connection
                assert(m_mapClients.find(pInfo->m_hConn) == m_mapClients.end());

                Printf("Connection request from %s", pInfo->m_info.m_szConnectionDescription);

                // A client is attempting to connect
                // Try to accept the connection.
                if (m_pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK) {
                    // This could fail.  If the remote host tried to connect, but then
                    // disconnected, the connection may already be half closed.  Just
                    // destroy whatever we have on our side.
                    m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
                    Printf("Can't accept connection.  (It was already closed?)");
                    break;
                }

                // Assign the poll group
                if (!m_pInterface->SetConnectionPollGroup(pInfo->m_hConn, m_hPollGroup)) {
                    m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
                    Printf("Failed to set poll group?");
                    break;
                }

                // Generate a random nick.  A random temporary nick
                // is really dumb and not how you would write a real chat server.
                // You would want them to have some sort of signon message,
                // and you would keep their client in a state of limbo (connected,
                // but not logged on) until them.  I'm trying to keep this example
                // code really simple.
                char nick[64];
                sprintf(nick, "BraveWarrior%d", 10000 + (rand() % 100000));

                // Send them a welcome message
                sprintf(temp, "Welcome, stranger.  Thou art known to us for now as '%s'; upon thine command '/nick' we shall know thee otherwise.", nick);
                SendStringToClient(pInfo->m_hConn, temp);

                // Also send them a list of everybody who is already connected
                if (m_mapClients.empty()) {
                    SendStringToClient(pInfo->m_hConn, "Thou art utterly alone.");
                } else {
                    sprintf(temp, "%d companions greet you:", (int)m_mapClients.size());
                    for (auto &c : m_mapClients)
                        SendStringToClient(pInfo->m_hConn, c.second.m_sNick.c_str());
                }

                // Let everybody else know who they are for now
                sprintf(temp, "Hark!  A stranger hath joined this merry host.  For now we shall call them '%s'", nick);
                SendStringToAllClients(temp, pInfo->m_hConn);

                // Add them to the client list, using std::map wacky syntax
                m_mapClients[pInfo->m_hConn];
                SetClientNick(pInfo->m_hConn, nick);
                break;
            }

            case k_ESteamNetworkingConnectionState_Connected:
                // We will get a callback immediately after accepting the connection.
                // Since we are the server, we can ignore this, it's not news to us.
                break;

            default:
                // Silences -Wswitch
                break;
        }
    }

    static ChatServer *s_pCallbackInstance;
    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pInfo) {
        s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
    }

    void PollConnectionStateChanges() {
        s_pCallbackInstance = this;
        m_pInterface->RunCallbacks();
    }
};

ChatServer *ChatServer::s_pCallbackInstance = nullptr;

/////////////////////////////////////////////////////////////////////////////
//
// ChatClient
//
/////////////////////////////////////////////////////////////////////////////

class ChatClient {
   public:
    void Run(const SteamNetworkingIPAddr &serverAddr) {
        // Select instance to use.  For now we'll always use the default.
        m_pInterface = SteamNetworkingSockets();

        // Start connecting
        char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
        serverAddr.ToString(szAddr, sizeof(szAddr), true);
        Printf("Connecting to chat server at %s", szAddr);
        SteamNetworkingConfigValue_t opt;
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void *)SteamNetConnectionStatusChangedCallback);
        m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 1, &opt);
        if (m_hConnection == k_HSteamNetConnection_Invalid)
            FatalError("Failed to create connection");

        while (!g_bQuit) {
            PollIncomingMessages();
            PollConnectionStateChanges();
            PollLocalUserInput();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

   private:
    HSteamNetConnection m_hConnection;
    ISteamNetworkingSockets *m_pInterface;

    void PollIncomingMessages() {
        while (!g_bQuit) {
            ISteamNetworkingMessage *pIncomingMsg = nullptr;
            int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
            if (numMsgs == 0)
                break;
            if (numMsgs < 0)
                FatalError("Error checking for messages");

            // Just echo anything we get from the server
            fwrite(pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout);
            fputc('\n', stdout);

            // We don't need this anymore.
            pIncomingMsg->Release();
        }
    }

    void PollLocalUserInput() {
        std::string cmd;
        while (!g_bQuit && LocalUserInput_GetNext(cmd)) {
            // Check for known commands
            if (strcmp(cmd.c_str(), "/quit") == 0) {
                g_bQuit = true;
                Printf("Disconnecting from chat server");

                // Close the connection gracefully.
                // We use linger mode to ask for any remaining reliable data
                // to be flushed out.  But remember this is an application
                // protocol on UDP.  See ShutdownSteamDatagramConnectionSockets
                m_pInterface->CloseConnection(m_hConnection, 0, "Goodbye", true);
                break;
            }

            // Anything else, just send it to the server and let them parse it
            m_pInterface->SendMessageToConnection(m_hConnection, cmd.c_str(), (uint32)cmd.length(), k_nSteamNetworkingSend_Reliable, nullptr);
        }
    }

    void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo) {
        assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

        // What's the state of the connection?
        switch (pInfo->m_info.m_eState) {
            case k_ESteamNetworkingConnectionState_None:
                // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
                break;

            case k_ESteamNetworkingConnectionState_ClosedByPeer:
            case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
                g_bQuit = true;

                // Print an appropriate message
                if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
                    // Note: we could distinguish between a timeout, a rejected connection,
                    // or some other transport problem.
                    Printf("We sought the remote host, yet our efforts were met with defeat.  (%s)", pInfo->m_info.m_szEndDebug);
                } else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
                    Printf("Alas, troubles beset us; we have lost contact with the host.  (%s)", pInfo->m_info.m_szEndDebug);
                } else {
                    // NOTE: We could check the reason code for a normal disconnection
                    Printf("The host hath bidden us farewell.  (%s)", pInfo->m_info.m_szEndDebug);
                }

                // Clean up the connection.  This is important!
                // The connection is "closed" in the network sense, but
                // it has not been destroyed.  We must close it on our end, too
                // to finish up.  The reason information do not matter in this case,
                // and we cannot linger because it's already closed on the other end,
                // so we just pass 0's.
                m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
                m_hConnection = k_HSteamNetConnection_Invalid;
                break;
            }

            case k_ESteamNetworkingConnectionState_Connecting:
                // We will get this callback when we start connecting.
                // We can ignore this.
                break;

            case k_ESteamNetworkingConnectionState_Connected:
                Printf("Connected to server OK");
                break;

            default:
                // Silences -Wswitch
                break;
        }
    }

    static ChatClient *s_pCallbackInstance;
    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pInfo) {
        s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
    }

    void PollConnectionStateChanges() {
        s_pCallbackInstance = this;
        m_pInterface->RunCallbacks();
    }
};

ChatClient *ChatClient::s_pCallbackInstance = nullptr;

void ChatClient_Init(SteamNetworkingIPAddr addrServer) {
    s_pThreadMain = new std::thread([addrServer]() {
        ChatClient client;
        client.Run(addrServer);
    });
}

const uint16 DEFAULT_SERVER_PORT = 27020;

void PrintUsageAndExit(int rc = 1) {
    fflush(stderr);
    printf(
        R"usage(Usage:
    example_chat client SERVER_ADDR
    example_chat server [--port PORT]
)usage");
    fflush(stdout);
    exit(rc);
}

int main(int argc, const char *argv[]) {
    if (argc == 1) {
        App app;
        app.run();
        return 0;
    }
    bool bServer = false;
    bool bClient = false;
    int nPort = DEFAULT_SERVER_PORT;
    SteamNetworkingIPAddr addrServer;
    addrServer.Clear();

    for (int i = 1; i < argc; ++i) {
        if (!bClient && !bServer) {
            if (!strcmp(argv[i], "client")) {
                bClient = true;
                continue;
            }
            if (!strcmp(argv[i], "server")) {
                bServer = true;
                continue;
            }
        }
        if (!strcmp(argv[i], "--port")) {
            ++i;
            if (i >= argc)
                PrintUsageAndExit();
            nPort = atoi(argv[i]);
            if (nPort <= 0 || nPort > 65535)
                FatalError("Invalid port %d", nPort);
            continue;
        }

        // Anything else, must be server address to connect to
        if (bClient && addrServer.IsIPv6AllZeros()) {
            if (!addrServer.ParseString(argv[i]))
                FatalError("Invalid server address '%s'", argv[i]);
            if (addrServer.m_port == 0)
                addrServer.m_port = DEFAULT_SERVER_PORT;
            continue;
        }

        PrintUsageAndExit();
    }

    if (bClient == bServer || (bClient && addrServer.IsIPv6AllZeros()))
        PrintUsageAndExit();

    // Create client and server sockets
    InitSteamDatagramConnectionSockets();
    LocalUserInput_Init();

    if (bClient) {
        ChatClient_Init(addrServer);
        App app;
        app.run();
    } else {
        ChatServer server;
        server.Run((uint16)nPort);
    }

    ShutdownSteamDatagramConnectionSockets();

    // Ug, why is there no simple solution for portable, non-blocking console user input?
    // Just nuke the process
    //LocalUserInput_Kill();
    NukeProcess(0);
}
