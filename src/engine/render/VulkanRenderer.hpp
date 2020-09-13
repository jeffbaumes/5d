#pragma once

#include "../core/glm_include.hpp"
#include "Vertex.hpp"

#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

/*

Plan

App::run
    World world;
    WorldView view(world);
    window = glfwInit ...
    surface = glfwCreateWindowSurface ...
    view.initSurface(surface);
    world.run();

Call tree

App::run x
  initWindow x
    glfwInit ... x
  initVulkan x
    Vulkan::init ... x
  createSurface
    glfwCreateWindowSurface x
    Vulkan::initSurface ... x
  initWorld x
    World::init ... x
    World::printStats ... x
    World::updateVulkan ... x
  mainLoop x
    glfwPollEvents ... x
    updateUniforms x
      updatePositions x
        World::getCell ... x
      World::updateUBO ... x
    Vulkan::draw ... x
    World::pollEvents ... x
  cleanup
    Vulkan::cleanup ... x
    glfwDestroyWindow ... x
    glfwTerminate ... x


World::init x
  Vulkan::setVerticesAndIndices ... x
World::getCell x
  getCellInChunk x
    Chunk::getCell ... x
World::updateVulkan x
  Vulkan::resetVertexRange ... x
World::pollEvents x
  Client::pollEvents ... x
  updateVulkan x
    Vulkan::resetVertexRange ... x
World::updateUBO x
  (empty)

Vulkan::init x
  initVulkan x
    createInstance x
      checkValidationLayerSupport x
      getRequiredExtensions x
      populateDebugMessengerCreateInfo x
    setupDebugMessenger x
      populateDebugMessengerCreateInfo x
      CreateDebugUtilsMessengerEXT x
Vulkan::initSurface x
  pickPhysicalDevice x
    isDeviceSuitable x
      findQueueFamilies x
      checkDeviceExtensionSupport x
      querySwapChainSupport x
      QueueFamilyIndices::isComplete x
    getMaxUsableSampleCount x
  createLogicalDevice x
    findQueueFamilies x
  createSwapChain x
    querySwapChainSupport x
    chooseSwapSurfaceFormat x
    chooseSwapPresentMode x
    chooseSwapExtent x
    findQueueFamilies x
  createImageViews x
    createImageView x
  createRenderPass x
    findDepthFormat x
      findSupportedFormat x
  createDescriptorSetLayout x
  createGraphicsPipeline x
    readFile x
    createShaderModule x
    Vertex::getBindingDescription x
    Vertex::getAttributeDescriptions x
  createCommandPool x
    findQueueFamilies x
  createColorResources x
    createImage x
      findMemoryType x
    createImageView x
  createDepthResources x
    findDepthFormat x
    createImage x
      findMemoryType x
    createImageView x
  createFramebuffers x
  createTextureImage x
    stbi_load x
    createBuffer x
      findMemoryType x
    createImage x
      findMemoryType x
    transitionImageLayout x
      beginSingleTimeCommands x
      endSingleTimeCommands x
    copyBufferToImage x
      beginSingleTimeCommands x
      endSingleTimeCommands x
    generateMipmaps x
      beginSingleTimeCommands x
      endSingleTimeCommands x
  createTextureImageView x
    createImageView x
  createTextureSampler x
Vulkan::draw x
  cleanupSwapChain x
  DestroyDebugUtilsMessengerEXT x
Vulkan::setVerticesAndIndices x
  createVertexBuffer x
    createBuffer x
    copyBuffer x
  createIndexBuffer x
    createBuffer x
    copyBuffer x
  createUniformBuffers x
    createBuffer x
  createDescriptorPool x
  createDescriptorSets x
  createCommandBuffers x
  createSyncObjects x
Vulkan::resetVerticesAndIndices x
  createVertexBuffer x
    copyBuffer x
  createIndexBuffer x
    copyBuffer x
Vulkan::resetVertexRange x
  createVertexBuffer x
    copyBuffer x
Vulkan::resetIndexRange x
  createIndexBuffer x
    copyBuffer x
Vulkan::cleanup x
  cleanupSwapChain x
  DestroyDebugUtilsMessengerEXT x
*/

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete();
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

const int MAX_ENTITIES = 10;

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 selectedCell;
    alignas(16) glm::vec3 xyz;
    alignas(8) glm::vec2 uv;
    alignas(8) glm::vec2 selectedCellUV;
    alignas(4) float uvView;
    alignas(16) glm::vec4 entityLocationUV[MAX_ENTITIES];
    alignas(16) glm::vec4 entityRotation[MAX_ENTITIES];
    alignas(16) glm::vec4 entityLocationXYZ[MAX_ENTITIES];
};

class VulkanRenderer {
   public:
    VulkanRenderer(std::vector<const char *> extensions);

    void initSurface(VkSurfaceKHR surface);
    void setVerticesAndIndices(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
    void draw();
    void resetVertexRange(const std::vector<Vertex> &vertices, size_t start, size_t size, size_t arrStart = 0);
    void cleanup();

    bool framebufferResized;

    int windowWidth;
    int windowHeight;

    VkInstance instance;
    UniformBufferObject ubo;
    VkExtent2D swapChainExtent;
  private:

    VkSurfaceKHR surface;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    uint32_t mipLevels;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    size_t indexCount;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    void cleanupSwapChain();
    void recreateSwapChain();
    void createInstance(std::vector<const char *> extensions);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void setupDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createColorResources();
    void createDepthResources();
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);
    void createTextureImage();
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
    VkSampleCountFlagBits getMaxUsableSampleCount();
    void createTextureImageView();
    void createTextureSampler();
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createVertexBuffer(const std::vector<Vertex> &vertices, bool update, size_t start, size_t size, size_t arrStart);
    void createIndexBuffer(const std::vector<uint32_t> &indices, bool update, size_t start, size_t size, size_t arrStart);
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createCommandBuffers();
    void createSyncObjects();
    void updateUniformBuffer(uint32_t currentImage);
    void drawFrame();
    VkShaderModule createShaderModule(const std::vector<char> &code);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    std::vector<const char *> getRequiredExtensions();
    bool checkValidationLayerSupport();
    static std::vector<char> readFile(const std::string &filename);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
    const std::string TEXTURE_PATH = "src/client/textures/merged.png";
    const std::string VERTEX_SHADER_PATH = "src/client/shaders/vert.spv";
    const std::string FRAG_SHADER_PATH = "src/client/shaders/frag.spv";
    const int MAX_FRAMES_IN_FLIGHT = 2;

    const std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation",
    };

    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
};
