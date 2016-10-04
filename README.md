# Distorted-Dungeon
3D rogue like/platformer game in development

## What's been done ?
  The dungeon is generated procedurally using Kruskal's algorithm to find computing the minimum spanning tree of a graph composed of rooms as nodes. Edges give the corridors between rooms.
  
  Game uses OpenGL v3.3/GLSL and is based on a Entity component system design pattern.
  Components that currently compose the game are :
  * AABB 3d box and 2d square Collisable
  * Movable which comprises the speed, direction, quaternion, and position of the entity
  * Renderable i.e. Mesh .obj, Line, Plane, Cube, Billboard, Skybox 
  * Carried  : all entitys which can be carried like weapons, gear, potions, misci etc
  * Container : all entitys which can carry entitys
  * Motion (declined in different pattern of motion like circular, A -> B etc...
  * Effect : all entitys that produces effects when we interact with it 

##Commands : (for azerty keybords) 
 * R : jump
 * A : Equip (.resp Remove) your axe (.resp from your hand).
 * ZQSD : Displacement
