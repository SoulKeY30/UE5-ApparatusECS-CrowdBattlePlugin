This project is the beta version, so bugs may occur.

The project itself is open sourced. However the project is based on the Apparatus ECS framework on FAB, which is not free.

This project also has dependency on the Flowfield Canvas plugin. The plugin is also a FAB plugin, but this project has included a free lite version of Flowfield Canvas.

This project relies on Anim To Texture plugin, which is an official plugin included in the engine.

This project is developed and tested by UE5.3 on Windows.

The performance test result is 60+fps running 10000 agents and 30+fps running 20000 agents on i9-9900k and rts2080ti, cpu capped.

This project uses ecs for logic and niagara+vat as the rendering solution.

This project uses rvo2 for avoidance/collision.

This project uses neighbour grid for collision detection and flow field for navigation.

THis project includes an editor utility widget that allows users to set up a new agent with only 1 click of a button.

Open for suggestions! Any bug reports or optimization suggestions are welcomed whole heartedly!
