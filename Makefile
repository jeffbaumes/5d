ifndef VULKAN_SDK_PATH
$(error $(VULKAN_SDK_PATH) Not defined. Source setup.sh)
endif

RUN_VARS = LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib
ifeq ($(OS),Windows_NT)
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Linux)
    RUN_VARS += VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d
  endif
  ifeq ($(UNAME_S),Darwin)
    RUN_VARS += VK_ICD_FILENAMES=$(VULKAN_SDK_PATH)/share/vulkan/icd.d/MoltenVK_icd.json
    RUN_VARS += VK_LAYER_PATH=$(VULKAN_SDK_PATH)/share/vulkan/explicit_layer.d
  endif
endif

GAME_NETWORKING_SOCKETS_PATH = /Users/jeff/code/GameNetworkingSockets

CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(GAME_NETWORKING_SOCKETS_PATH)/include -Ilib/vendor -g
# CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -Ilib/vendor -O3 -DNDEBUG
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

HFILES = src/VulkanUtil.hpp src/World.hpp
CFILES = src/main.cpp src/VulkanUtil.cpp src/World.cpp
SFILES = src/shaders/frag.spv src/shaders/vert.spv

VulkanTest: $(HFILES) $(CFILES) $(SFILES)
	g++ $(CFLAGS) -o VulkanTest $(CFILES) $(LDFLAGS)

src/shaders/frag.spv: src/shaders/shader.frag
	$(VULKAN_SDK_PATH)/bin/glslc $< -o $@

src/shaders/vert.spv: src/shaders/shader.vert
	$(VULKAN_SDK_PATH)/bin/glslc $< -o $@

.PHONY: test clean

test: VulkanTest
	$(RUN_VARS) ./VulkanTest

clean:
	rm -f VulkanTest
