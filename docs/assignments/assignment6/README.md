# Assignment 6：BVH 加速光线追踪

## 效果展示

![Assignment 6：Bunny 光线追踪渲染结果](assignment6.png)

使用课程提供的 Bunny 网格和逐三角形几何法线着色，因此表面保留明显的三角形明暗分面。

## 实现说明

在课程框架基础上补全四处实现：

- `Renderer::Render`：从像素中心生成归一化主光线，调用光线追踪并写入帧缓冲。
- `Bounds3::IntersectP`：使用 slab 方法求三个轴的有效交点区间，处理平行光线、边界相切和零厚度包围盒。
- `BVHAccel::GetIntersection`：未命中包围盒时剪枝，叶节点查询图元，两侧子树都命中时返回最近交点。
- `Triangle::GetIntersection`：沿用框架的 Möller–Trumbore 计算及背面剔除，排除无效交点并填充位置、法线、距离、对象和材质引用。

场景和网格各自持有 BVH，支持 `Naive` 中点划分与 `Sah` 表面积启发式划分。作业要求见 [PDF](PA6.pdf)。

## SAH 划分

SAH（Surface Area Heuristic，表面积启发式）通过估算光线求交的平均成本选择 BVH 划分。以子包围盒面积与父包围盒面积的比值估计访问子节点的概率，成本模型为：

```text
C = Ct + Ci × (A_left × N_left + A_right × N_right) / A_parent
```

`Ct` 是遍历成本，`Ci` 是单个图元求交成本，`A` 表示包围盒表面积，`N` 表示子节点内的图元数量。两个子包围盒可能重叠，因此面积比不要求相加等于 1。模型背景见 [PBRT：Surface Area Heuristic](https://www.pbr-book.org/4ed/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies#TheSurfaceAreaHeuristic)。

本实现使用 Sweep SAH：

1. 分别沿 X、Y、Z 轴按图元包围盒质心排序，在相邻且不同的质心之间考虑划分。
2. 通过前缀、后缀包围盒累积计算所有候选的面积与图元数量，选择估算成本最低的轴和位置。
3. 同一父节点下，遍历成本、图元求交成本及父包围盒面积是公共常量，比较时只计算 `A_left × N_left + A_right × N_right`。
4. 成本相同时优先更均衡的划分；质心全部重合或父包围盒面积退化时回退到中点划分，确保两侧均非空。

沿用框架的单图元叶节点结构，SAH 用于选择划分，不进行“多个图元直接组成叶节点”的成本比较。每个节点的候选评估包含三个轴的排序和线性扫描；构建通常比简单中点划分更费时，目标是减少后续查询的访问成本，实际收益需测量。

`main.cpp` 中的 `split_method` 默认设为 `BVHAccel::SplitMethod::Sah`，同时传给 Bunny 网格和场景。改为 `BVHAccel::SplitMethod::Naive` 可使用原来的中点划分。终端分别输出网格、场景的 BVH 构建耗时，以及渲染耗时。

尚未执行两种模式的性能对比，不提供未经测量的加速比。文档中的结果图来自之前的中点划分版本；SAH 改变加速结构，不改变场景和着色算法。

## 运行

程序无需交互，在当前工作目录生成 `binary.ppm`（P6 RGB 格式，1280 × 960），可用支持 PPM 的图片查看器打开。

模型路径通过仓库的 `ASSETS_PATH` 定位到 `assets/models/bunny/bunny.obj`，无需复制到运行目录。

构建与运行方法见 [Windows 构建指南](../../BUILD.md)，目标名为 `assignment6`。上方 PNG 用于文档展示，程序仍输出 PPM。
