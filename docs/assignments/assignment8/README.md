# Assignment 8：质点弹簧绳子模拟

## 当前状态

已迁移课程框架并适配本仓库，保留 `rope.cpp` 中的物理模拟 TODO，尚未实现绳子模拟。作业要求见 [PDF](assignment8.pdf)。

待实现内容包括绳子的节点和弹簧初始化、固定节点设置、Hooke 定律、Euler 积分、Verlet 积分及阻尼。构造函数尚未创建节点，因此当前框架不会绘制绳子。

## 代码与依赖

- `src/assignment8/`：作业代码，文件名、命名和格式遵循仓库规范。作业类型位于 `assignment8` 命名空间；覆盖 CGL 虚函数的 `init`、`render`、`resize`、`name`、`info`、`key_event` 保留接口名称。
- `third_party/cgl/`：课程自带的窗口、绘制及文字显示框架，包括 GLFW、GLEW 和字体数据，保留其源码格式及接口，仅适配构建。来源与改动见 [CGL 说明](../../../third_party/README.md#cgl)。
- `third_party/freetype/`：FreeType 2.15.0（`VER-2-15-0`），用于 CGL 的屏幕文字，与 GLFW、GLEW 一起从源码构建为静态库，无需另外安装。来源和构建选项见 [FreeType 说明](../../../third_party/README.md#freetype)。OpenGL 通过系统开发环境提供。

绳子通过 `std::unique_ptr` 持有质点和弹簧，弹簧端点为不持有所有权的 `Mass*`。实现初始化 TODO 时使用 `std::make_unique<Mass>`、`std::make_unique<Spring>`；连接端点时通过质点指针的 `.get()` 获取地址。

`Application` 持有两根绳子；CGL Viewer 在析构时释放 Application，入口代码按此约定转移所有权。命令行解析使用标准 C++，不依赖原框架的 `unistd.h/getopt`，并修正了 `-g` 参数和键盘回调接口。

## 构建与运行

目标名为 `assignment8`，与其他作业使用相同的配置和构建流程；所需第三方库由 CMake 自动从源码构建。Windows 构建命令见 [构建指南](../../BUILD.md)，将示例中的目标名替换为 `assignment8` 即可。

运行示例：

```shell
build\msvc-release-local\bin\assignment8.exe -m 1 -g 0 -1 -s 64
```

| 参数 | 含义 | 默认值 |
| --- | --- | --- |
| `-m <FLOAT>` | 每个质点的质量，必须大于零 | `1` |
| `-g <FLOAT> <FLOAT>` | 重力加速度的 X、Y 分量 | `(0, -1)` |
| `-s <INT>` | 每帧模拟子步数，必须为正整数 | `64` |
| `-h` / `--help` | 显示命令行帮助 | — |

沿用框架默认配置：两根绳子的端点为 `(0, 200)` 与 `(-400, 200)`，各有 `3` 个节点，弹簧刚度为 `100`，固定节点索引为 `0`；这些设置会在完成构造函数后生效。

蓝色用于 Euler 模拟，绿色用于 Verlet 模拟；`-` 键将每帧子步数减半，`=` 键将其加倍，`Esc` 关闭窗口，反引号键切换屏幕信息显示。

沿用课程框架的时间步设置：每帧调用指定次数的模拟函数，每个子步的 `delta_t = 1 / steps_per_frame`，没有按真实帧间耗时推进模拟。程序使用交互窗口，不会自动保存结果图片，也不需要额外复制模型或字体资源。
