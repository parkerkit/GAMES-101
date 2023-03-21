# 第三方库

本目录保存课程框架使用的第三方代码。各组件保留原有版权署名和许可，不适用本仓库原创代码的 MIT 许可声明。课程入口见 [GAMES101](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)。

## OBJ Loader

`obj_loader/OBJ_Loader.h` 随 GAMES101 Assignment 3 框架迁入，保留原有署名。文件头注明作者为 Robert Smith、来源为 [Bly7/OBJ-Loader](https://github.com/Bly7/OBJ-Loader)，并标注使用 MIT 许可。

本地修改：将顶点格式变量 `vtype` 初始化为 `0`，避免未知格式触发未初始化读取；合法格式的解析逻辑保持不变。

## CGL

`cgl/` 随 GAMES101 Assignment 8 课程框架迁入，包含 CGL、内嵌字体、GLFW 和 GLEW。保留框架的目录结构、源码格式、接口名称及已有版权署名。

### 本地构建适配

仅修改两处原有构建文件：

- `cgl/CMakeLists.txt`：替换原框架的全局构建配置，直接构建 CGL 静态库及内嵌字体；GLEW 只编译 `glew.c`，GLFW 使用自带构建脚本。链接仓库内构建的 FreeType，通过 CMake target 传递 OpenGL、FreeType、线程库及静态 GLEW 的编译定义，关闭第三方示例、测试、文档和安装目标。MSVC 下启用 UTF-8 源码读取，并提供 `_USE_MATH_DEFINES`、`NOMINMAX` 和必要的 CRT 编译定义。
- `cgl/deps/glfw/CMakeLists.txt`：将 CMake 最低版本调整为仓库使用的 `3.25`，移除原脚本对 `CMP0042` 的旧策略设置；平台源码选择逻辑保留。

FreeType 由 `src/assignment8/CMakeLists.txt` 先行接入，版本及配置见下方 [FreeType 说明](#freetype)。CGL 不再通过 `find_package(Freetype)` 查找本地安装。

CGL 本体沿用原来的 C++11 标准，作业代码使用仓库的 C++20。原有 `cgl/src/CMakeLists.txt` 和 GLEW 构建脚本作为框架材料保留，当前接入方式不调用它们。

第三方源码没有应用仓库格式化或命名规则，也没有接入作业代码的 clang-tidy 设置。源文件、头文件中的既有逻辑保留。

### 来源与许可

CGL 原构建文件保留了 CMU462 Fall 2015 的版权署名，所附第三方代码保留各自声明：

- GLFW：[COPYING.txt](cgl/deps/glfw/COPYING.txt)。
- GLEW：[LICENSE.txt](cgl/deps/glew/LICENSE.txt)。
- 其他组件及内嵌资源以各文件内的原有声明为准；课程提供的目录中没有单独的 CGL 总体许可证文件。

## FreeType

`freetype/` 使用 FreeType 2.15.0，源码来自官方仓库的 [VER-2-15-0 tag](https://github.com/freetype/freetype/tree/VER-2-15-0)，用于 Assignment 8 的 CGL 屏幕文字显示。

上游说明见 [FreeType README](freetype/README)。源码、格式及原有构建文件保持不变。

### 构建接入

`src/assignment8/CMakeLists.txt` 通过 `add_subdirectory` 调用 FreeType 自带的 CMake，随作业 8 一起构建：

- 构建静态库，不使用预编译库，也不需要 FreeType DLL。
- 使用与作业一致的编译器、架构和 Debug/Release 配置。
- MSVC 编译 FreeType 的 C 源码时指定 `/utf-8`，避免按本地代码页读取 UTF-8 文件；该参数不传给 Windows 资源编译器。
- 将上游 `freetype-interface` target 别名设为 `Freetype::Freetype`，供 CGL 链接；头文件路径和生成的配置头文件由 target 自动传递。
- 关闭外部 Zlib、BZip2、PNG、HarfBuzz、Brotli 和 HVF 支持，避免引入当前字体显示不需要的外部依赖。其中 `FT_DISABLE_ZLIB=ON` 表示使用 FreeType 自带的 zlib 实现。
- 设置 `SKIP_INSTALL_ALL=ON`，只随作业构建，不添加 FreeType 安装规则。

编译产物和生成的配置头文件位于构建目录中，不写回第三方源码目录。第三方源码不应用本仓库的格式化和命名规则。

### 来源与许可

本项目使用 FreeType 字体渲染库，保留上游版权和许可文件，以 [FreeType License](freetype/docs/FTL.TXT) 使用该库。完整许可说明见 [LICENSE.TXT](freetype/LICENSE.TXT)，各组件的独立声明以对应源文件为准。
