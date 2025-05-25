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
  - 60 fps with 30,000 agents
  - 30 fps with 60,000 agents
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
- **AI Perception**:
  - Support vision. Agent can do sector trace for targets, and optionally those not blocked by obstacles
- **BP WorkFlow**:
  - Spawning agents by agent config data asset
  - Getting and setting trait values to change agents' behaviors runtime
  - Trace for agents and apply damage and debuff
  - Easy set up actors or characters that can be detected and attacked by agents
- **NS GPU Particle VAT**:
  - Basic A to B animation blending
  - Very handy editor utility widget to set up VAT in 1 click
- **NS GPU Particle UI**: 
  - Health Bar, Text Pop, Ground Ring
- **NS CPU Batched Emitter**:
  - Batched particle burst, Batched trails
  
## Roadmap

In progress:
1. Tower defence demo map
2. Agent individual navigation
3. Sprite Rendering Support
4. TurboSequence GPU SKM Support
6. More trace shapes
7. Agent Formation
8. Network replication
9. RTS demo map

## Leave a star if you find this project useful ;)
