# ApparatusECS-CrowdBattleFramePlugin

[![Join Discord](https://img.shields.io/badge/Discord-Join%20Chat-blue?logo=discord)](https://discord.gg/8AUMxq3SgV)

## Dependencies

This plugin is open source, but has the following dependencies:

- **Apparatus ECS Framework** (Paid on FAB)  
  https://www.fab.com/listings/23ddc9c0-a218-44ed-8c0c-ebef362f08d5

- **Flowfield Canvas** (Pathfinding plugin)  
  A lite version is included with basic functionality. Full version available for purchase:  
  https://www.fab.com/listings/e306cd3d-9855-45bf-a978-d9ac6ae2ee33

- **Anim To Texture** (Official Unreal Engine plugin)  
  Included with the engine

## Technical Details

- **Status**: Beta version (may contain bugs)
- **Tested Platforms**: 
  - Windows
  - UE5.3, UE5.4, UE5.5
- **Performance**: 
  - 60fps with 10,000 agents
  - 30fps with 30,000 agents
  - (Tested on AMD Ryzen 5900X + RTX 4080S, CPU capped)
- **Technology Stack**:
  - ECS for logic
  - Niagara + VAT for rendering
  - RVO2 for avoidance/collision
  - Neighbour grid for collision detection
  - Flow field for navigation
- **Editor Features**:
  - Includes utility widget for one-click agent setup

## Features

Comprehensive crowd battle system including most commonly needed features.  
Demo maps are included in the plugin's content folder.

## Roadmap

Coming in future updates:
1. 2D atlas renderer and converter
2. Agent individual navigation
