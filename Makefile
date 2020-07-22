ifndef VULKAN_SDK_PATH
$(error $(VULKAN_SDK_PATH) Not defined. Source setup.sh)
endif

CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -Ilib/vendor -g
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

VulkanTest: src/main.cpp src/VulkanUtil.cpp src/VulkanUtil.hpp
	g++ $(CFLAGS) -o VulkanTest src/main.cpp src/VulkanUtil.cpp $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d ./VulkanTest

clean:
	rm -f VulkanTest
