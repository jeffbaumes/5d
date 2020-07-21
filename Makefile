VULKAN_SDK_PATH = /Users/jeff/code/planar/vulkan/macOS

CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

VulkanTest: main.cpp
	clang++ $(CFLAGS) -o VulkanTest main.cpp $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_ICD_FILENAMES=$(VULKAN_SDK_PATH)/share/vulkan/icd.d/MoltenVK_icd.json VK_LAYER_PATH=$(VULKAN_SDK_PATH)/share/vulkan/explicit_layer.d ./VulkanTest

clean:
	rm -f VulkanTest
