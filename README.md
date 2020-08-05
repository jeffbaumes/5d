# 5d

## Development

To clone:

```
git clone git@github.com:jeffbaumes/5d.git --recurse-submodules
```

If you already pulled but forgot to add `--recuse-submodules` pull the submodules with:

```
git submodule update --init
```

Set up your development environment by following the instructions in the
[Vulkan tutorial](https://vulkan-tutorial.com/Development_environment).
You will also need the dependencies for
[GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/BUILDING.md).

To build:
```
mkdir build
cd build
ccmake ..    # Press 'c', set VULKAN_SDK_PATH appropriately, then press 'g' to generate
make -j4     # Or run whatever other build system you set up
```

To run, first set your Vulkan path:
```
export VULKAN_SDK_PATH=Your Vulkan SDK path (include up to vulkan/[architecture])
```

Then for Mac:
```
export LD_LIBRARY_PATH=$VULKAN_SDK_PATH/lib
export VK_LAYER_PATH=$VULKAN_SDK_PATH/share/vulkan/explicit_layer.d
export VK_ICD_FILENAMES=$VULKAN_SDK_PATH/share/vulkan/icd.d/MoltenVK_icd.json
```

For Linux set these instead:
```
export LD_LIBRARY_PATH=$VULKAN_SDK_PATH/lib
export VK_LAYER_PATH=$VULKAN_SDK_PATH/etc/vulkan/explicit_layer.d
```

Finally, from the `build/` directory:
```
src/5d
```

To conform to the coding style, set your vscode setting:
```
C_Cpp: Clang_format_fallback Style
```

to
```
{ BasedOnStyle: Google, IndentWidth: 4, ColumnLimit: 0 }
```

Until someone finishes [clang-format dangling parentheses](https://reviews.llvm.org/D33029)
we will stick with long lines.
