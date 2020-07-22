# 5d

## Development

Set up your development environment by following the instructions in the
[Vulkan tutorial](https://vulkan-tutorial.com/Development_environment).

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
