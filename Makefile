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

CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -Ilib/vendor -g
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

VulkanTest: src/main.cpp src/VulkanUtil.cpp src/VulkanUtil.hpp
	g++ $(CFLAGS) -o VulkanTest src/main.cpp src/VulkanUtil.cpp $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	$(RUN_VARS) ./VulkanTest

clean:
	rm -f VulkanTest
