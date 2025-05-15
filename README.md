# UE5-ApparatusECS-CrowdBattleFramePlugin

[![Join Discord](https://img.shields.io/badge/Discord-Join%20Chat-blue?logo=discord)](https://discord.gg/8AUMxq3SgV)
[![QQ Group](https://img.shields.io/badge/QQ%20Group-916358710-blue?logo=tencentqq)](https://jq.qq.com/?_wv=1027&k=5R5X5wX)

## Version
The repository version is my current progress. 
If it failed to compile, use the release version instead.

## Dependencies

This plugin is open source, but has the following dependencies:

- **Apparatus ECS Framework** (Paid on FAB)  
  https://www.fab.com/listings/23ddc9c0-a218-44ed-8c0c-ebef362f08d5

- **Flowfield Canvas** (Pathfinding plugin)  
  Since FFC is my plugin, I've included a lite version for free.
  Full version available for purchase:  
  https://www.fab.com/listings/e306cd3d-9855-45bf-a978-d9ac6ae2ee33

- **Anim To Texture** (Official Unreal Engine plugin)  
  Included with the engine

## Technical Details

- **Status**: Beta (with frequent modifications and may contain bugs)
- **Tested Platforms**: 
  - Windows
  - UE5.3, UE5.4, UE5.5
- **Performance**: 
  - 60fps with 15,000 agents
  - 30fps with 30,000 agents
  - (AMD Ryzen 5900X + RTX 4080S, CPU capped. Shipping Package)
- **Technology Stack**:
  - ECS for logic
  - Niagara + VAT for rendering
  - RVO2 for avoidance/collision
  - Neighbour grid for collision detection
  - Flow field for navigation

## Features

Comprehensive crowd battle system including many commonly needed features. Demo maps are included in the plugin's content folder.

- **AI Behaviors**: 
  - Birth, Sleep, Patrol, Chase, Attack, Hit, Death, each with many params for fine tuning
  - Debuffs, including kinetic launching, ice slowing and fire temporal dmg, with corresponding material Fx.
  - 2.5D movement supporting flying, falling and moving on uneven surfaces
  - Navigation and avoidance support sphere and box obstacles
- **BP WorkFlow**: 
  - Getting and setting trait values to change agents' behaviors runtime
  - Trace agents and apply damage and debuff to them
  - Easy set up actors or characters that can be detected and attacked by agents
- **AI Perception**:
  - Support vision. Agent can do sector trace for targets, and optionally those not blocked by obstacles
- **NS GPU Particle UI**: 
  - Health Bar, Text Pop, Ground Ring
- **NS GPU Particle VAT**:
  - Basic A to B animation blending
  - Very handy editor utility widget to set up VAT in 1 click
- **Batched Fx**:
  - Batched particle burst, Batched trails
  
## Roadmap

In progress:
1. Tower defence demo map
2. Agent individual navigation
3. Sprite Rendering Support
4. TurboSequence GPU ISM Rendering Support
6. More trace shapes
7. Agent Formation
8. Network replication
9. RTS demo map

## Leave a star if you find this project useful ;)


# UE5-ApparatusECS-群战框架插件

## 依赖项

本插件为开源项目，但需要以下依赖：

- **Apparatus ECS框架** (FAB平台付费)  
  https://www.fab.com/listings/23ddc9c0-a218-44ed-8c0c-ebef362f08d5

- **流场画布** (寻路插件)  
  由于FFC是作者自有插件，已内置精简版免费提供  
  完整版购买链接：  
  https://www.fab.com/listings/e306cd3d-9855-45bf-a978-d9ac6ae2ee33

- **动画转贴图** (官方UE插件)  
  引擎自带

## 技术规格

- **当前状态**：测试版（频繁修改中，可能存在bug）
- **测试平台**：
  - Windows
  - UE5.3/UE5.4/UE5.5
- **性能表现**：
  - 15,000单位：60帧
  - 30,000单位：30帧
  - (测试环境：AMD锐龙5900X + RTX4080S，CPU瓶颈.Shipping模式打包)
- **技术栈**：
  - ECS架构处理逻辑
  - Niagara+VAT实现渲染
  - RVO2避障/碰撞系统
  - 邻居网格碰撞检测
  - 流场导航
- **编辑器功能**：
  - 包含一键式单位设置工具控件

## 功能特性

完整的群战系统，包含大多数常用功能。  
插件内容文件夹中附带了演示地图。

## 开发路线

未来版本将新增：
1. 2D图集渲染器与转换工具
2. 单位独立导航功能
3. 更多追踪形状支持
4. 阵型功能

## 如果觉得项目有用，请点个星星哦 ;)
