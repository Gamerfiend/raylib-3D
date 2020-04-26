# raylib-3D
An extension library for the amazing raylib library, that adds better support for 3D. The Deferred Rendering is based on the work done by @TheLumaio, but moved to a single header format with functions and data structures renamed to better follow the raylib code style.

Current Implementation Plan:
- [x] Deferred Rendering
- [ ] Model, Material, Animation loading through Assimp
- [ ] Skeletal Animation playback

Wherever possible, this extension library will follow the raylib paradigm in both naming convention and ease of use. Users should be able to include this with raylib, and have interoperability. 

**Special Thanks:**
- The Lumaio, [for work on GBuffer implementation using raylib](https://github.com/TheLumaio/Raylib-GBuffers)
- Raysan, for the creation of raylib!


## Usage
The examples are a good place to start when wanting to see this library extension in action!

You'll need to have the includes folder in your project directory, as R3D uses glad for OpenGL profile loading and stb_image for texture loading in assimp support.

There are some configurable options that can be defined before the implementation and inclusion of the header file.
```c
#define R3D_ASSIMP_SUPPORT // Enable assimp support
```
By enabling assimp support, you must both have assimp installed and include it when building your project.


To get started, you'll need to define `R3D_IMPLEMENTATION` in **one** source file. You can use the header anywhere else, but one and only one source file can have the `R3D_IMPLEMENTATION` define.
```c
#define R3D_IMPLEMENTATION
#include "r3d.h"
```
