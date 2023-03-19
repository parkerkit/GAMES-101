# Assignment 7：路径追踪

## 当前状态

已迁移课程原始框架并适配本仓库，尚未实现路径追踪。作业要求见 [PDF](Assignment7.pdf)。

保留原始框架的四处 TODO，未自动带入作业 6 的答案：

- `Bounds3::IntersectP`：射线与包围盒求交，可从作业 6 迁入。
- `BVHAccel::GetIntersection`：BVH 遍历及最近交点查询，可从作业 6 迁入。
- `Triangle::GetIntersection`：补全有效交点判断和交点信息，可从作业 6 迁入；作业 7 的 `Intersection` 还包含 `emit` 字段。
- `Scene::CastRay`：实现路径追踪，包括面光源的直接光照、间接光照和 Russian Roulette。

TODO 使用 `false`、空交点或零颜色作为占位返回值，避免原始空函数缺少返回值；当前运行只会输出黑图。

框架已提供三角形、网格及 BVH 的面积累积与表面采样接口，漫反射材质的 `Sample`、`Pdf`、`Eval` 接口，以及 `Scene::SampleLight`。场景和网格 BVH 沿用原框架的 `Naive` 划分，未迁入作业 6 的 SAH 扩展。

## 场景与运行

默认场景为 Cornell Box，包括红绿墙面、两个盒子和顶部发光网格。模型位于 `assets/models/cornellbox/`，通过 `ASSETS_PATH` 定位，无需复制到运行目录；OBJ 加载器复用 `third_party/obj_loader/`。

- 分辨率：`784 × 784`，在 `main.cpp` 中设置。
- 相机：位置 `(278, 273, -800)`，朝向正 Z 轴，垂直视场角 `40°`。
- 每像素采样次数：`16 SPP`，在 `renderer.cpp` 的 `spp` 中调整。
- Russian Roulette 继续概率：`0.8`，通过 `Scene::GetRussianRoulette()` 获取。
- 输出：当前工作目录的 `binary.ppm`，P6 RGB 格式，沿用框架的 `0.6` 次幂输出变换。

材质由 `main.cpp` 持有，网格引用材质，场景引用网格；使用场景期间需保持这些对象有效。

构建与运行方法见 [Windows 构建指南](../../BUILD.md)，目标名为 `assignment7`。结果展示将在完成实现后补充。
