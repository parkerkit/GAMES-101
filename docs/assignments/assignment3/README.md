# Assignment 3：着色与纹理映射

## 运行

不带参数运行进入交互模式，按 `a` / `d` 旋转，按 `Esc` 退出。命令行模式接受输出文件名和可选的着色模式：

```powershell
build\msvc-release-local\bin\assignment3.exe output.png normal
```

着色模式支持 `normal`、`phong`、`texture`、`bump`、`displacement`，默认 `phong`。图片写入当前工作目录。

模型和纹理复用 `assets/models/spot/`，路径由 CMake 定义，无需复制到可执行文件目录。
