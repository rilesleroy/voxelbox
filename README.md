![voxelbox logo](images/logo.png)
# Voxel Box
This was a small project I built in order to learn more about OpenGL, vertex buffers, voxels, and procedural mesh generation. Minecraft was a huge inspiration growing up and I wanted take a crack at solving the problem of terrain generation.

## Goals of the project
- To learn and get familiar with some of the lower level graphics technologies like OpenGL
- Learn the basics of lighting shaders
- learn how to optimize a solution from "first principles"

## How the project went
Overall I was happy with how this project turned out. I started out by rendering a single cube with a simple fps camera and basic ambient lighting. I had also implemented specular. Then I moved on to render and 8x8x8 chunk of cubes. worked fine. Then I wanted to render an 8x8x1 flat grid of chunks. Not so great and super slow. Why? draw calls.... a whole lot of draw calls....
You see for every single one of these 8^4 cubes was a draw call. Super slow.

First I wanted to just hack way 8^3 of them by just using a draw call per chunk rather than for every single cube in that chunk. To do this I basically just dump a copy of the original cube vertex data offsetting the position of each offset to a vertex buffer for the chunk for the chunk. That gave a fairly decent speed up, not bad but I'm still rendering a way more triangles I than I need.

Finally I added a check for each side of the a cube based on the voxel "density" of the cubes around it, to determine whether that size would be pushed to or not. By this point I wanted to wrap up the project so I decided to pull in some simple Perlin noise code and use to drive the density of the chunks.


![screenshot](images/screenshot.png)