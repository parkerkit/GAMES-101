# Windows 构建与运行

## 1. 环境准备

- Windows x64，Visual Studio / Build Tools 的“使用 C++ 的桌面开发”组件，支持 C++20。
- CMake 3.25+ 和 Ninja。
- [OpenCV](https://opencv.org/releases/) Windows 安装包，以及 [Eigen](https://gitlab.com/libeigen/eigen)。

Eigen 是头文件库，可通过 CMake 安装头文件及配置文件：

```shell
cmake -S <eigen-source> -B <eigen-build> -DCMAKE_INSTALL_PREFIX=<eigen-install>
cmake --install <eigen-build>
```

## 2. 配置依赖路径

在仓库根目录创建 `CMakeUserPresets.json`，参考以下示例修改路径；已有配置时按需合并。该文件已被 Git 忽略。

```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "env-local-deps",
      "hidden": true,
      "cacheVariables": {
        "OpenCV_DIR": "D:/opencv/build/x64/vc16/lib",
        "Eigen3_DIR": "D:/eigen/install/share/eigen3/cmake"
      }
    },
    {
      "name": "msvc-debug-local",
      "inherits": ["msvc-debug", "env-local-deps"]
    },
    {
      "name": "msvc-release-local",
      "inherits": ["msvc-release", "env-local-deps"]
    }
  ],
  "buildPresets": [
    {
      "name": "msvc-debug-local",
      "configurePreset": "msvc-debug-local"
    },
    {
      "name": "msvc-release-local",
      "configurePreset": "msvc-release-local"
    }
  ]
}
```

- `OpenCV_DIR` 指向包含 `OpenCVConfig.cmake` 的目录。
- `Eigen3_DIR` 指向包含 `Eigen3Config.cmake` 的目录。
- 路径以实际安装位置为准，建议使用 `/`；依赖库应与编译器的架构和构建配置匹配。

## 3. 构建与运行

打开 **x64 Native Tools Command Prompt for VS**，进入仓库根目录后执行：

```shell
cmake --preset msvc-release-local
cmake --build --preset msvc-release-local --target assignment0
build\msvc-release-local\bin\assignment0.exe
```

Debug 构建将命令和程序路径中的 `msvc-release-local` 替换为 `msvc-debug-local`。不指定 `--target assignment0` 时构建所有已接入的作业。

以上以 `assignment0` 检查 OpenCV/Eigen 环境。构建和运行其他作业时，将命令中的目标名和可执行文件名替换为对应的 `assignmentN`（`N` 为作业编号）。已接入的作业及其运行说明见[根目录 README](../README.md#作业进度)。

## 4. DLL 与常见问题

运行前，将 OpenCV 的 `bin` 目录加入当前终端的 `PATH`，或手动将所需 DLL 复制到可执行文件所在目录。DLL 应与链接库的版本、架构及 Debug/Release 配置匹配。

- 找不到 `cl`：使用上述 MSVC x64 开发者终端。
- 找不到 Ninja：安装 Ninja 并加入 `PATH`。
- 找不到 OpenCV/Eigen：检查配置路径是否包含对应的 CMake 配置文件。
- DLL 加载失败：检查版本、x64 架构及 Debug/Release 是否匹配。
