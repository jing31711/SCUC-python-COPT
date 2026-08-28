---
name: copt-model-crosscheck-workflow
language: zh-CN
description: "使用场景：当你需要通过对照 examples 中多语言官方示例（python/c/cpp/java/csharp/fortran）来校验 COPT 建模代码是否正确。"
---

# COPT 建模多语言对照校验工作流

## 目标

提供一套可重复执行的方法，用于验证基于 COPT 的建模代码在以下方面是否正确：

- API 调用正确

并通过与官方多语言示例对照来定位偏差。

## 适用场景

- 你新写或修改了 COPT 建模/求解代码，需要做正确性校验。
- 模型“能跑”但输出日志行为异常。
- 需要把代码修正到官方示例推荐的 API 形态。

## 输入信息

- 目标实现文件与运行命令。
- 预期行为：
  - 可行案例：应得到最优或可接受求解状态与结果。
  - 不可行案例：应触发 IIS 计算并落盘 IIS 文件。
- 对照目录：
  - examples/python
  - examples/c
  - examples/cpp
  - examples/java
  - examples/csharp
  - examples/fortran
- COPT 版本与运行环境（例如 conda 环境名）。

## 跨仓库路径配置策略

- 统一定义 `EXAMPLE_ROOT`，按以下优先级解析：
  1. 命令行参数（例如 `--example-root`）
  2. 环境变量 `COPT_EXAMPLES_ROOT`
  3. 仓库默认路径 `./copt-model-crosscheck-workflow/examples`
- 各语言目录统一从 `EXAMPLE_ROOT` 派生：
  - `${EXAMPLE_ROOT}/python`
  - `${EXAMPLE_ROOT}/c`
  - `${EXAMPLE_ROOT}/cpp`
  - `${EXAMPLE_ROOT}/java`
  - `${EXAMPLE_ROOT}/csharp`
  - `${EXAMPLE_ROOT}/fortran`
- 若必须目录不存在，应快速失败并给出缺失路径。
- 在验证报告中打印最终解析出的 `EXAMPLE_ROOT`，便于复现。


## 工作流

1. 建立 API 对照表
- 从官方示例提取关键 API：建模、求解、状态判断、IIS 计算、IIS 导出、模型导出、feasrelax等其余API。
- 先解析 `EXAMPLE_ROOT`，并记录本次实际使用的示例目录。
- 至少对照 Python + 一门编译型语言。
- 验收标准：形成一张简短 API 对照表。

2. 先校验状态语义
- 检查目标语言是否使用官方推荐的状态属性与枚举。
- 避免无依据混用属性、解状态属性、目标属性。
- 验收标准：所有分支条件都可映射到官方示例枚举语义。

3. 出现偏差时回查同类官方示例
- 若行为不一致，优先查对应功能示例（IIS、feasrelax、回调等）。
- 代码尽量向官方示例的 API 形态收敛。
- 验收标准：每个偏差都有“为何偏离/为何修复”的明确说明。

4. 输出验证记录
- 记录校验范围、修改点、运行命令、状态结果、工件路径。
- 验收标准：团队成员可据此独立复现。

## 固定输出格式

- 校验范围
- API 对照表（目标语言 vs 至少一种示例语言）
- 问题清单（按严重级别）
- 修复说明
- 验证命令
- 验证证据
- 剩余风险

## 质量检查清单

- 状态判断基于官方示例语义。
- 求解后已校验工件存在与非空。
- 文档中至少包含一次跨语言示例对照结论。


## 边界

- 本技能用于验证 API 与建模控制流正确性。
