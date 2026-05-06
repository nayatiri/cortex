# cortex engine

![Showcase](github_files/github_showcase.gif)
![PhysicsShowcase](github_files/physics_showcase.gif)

A powerful and lightweight 3D renderer built in C++ and OpenGL, supporting a wide range of features listed down below.

The focus of this project is to have an agile engine, i can use to develop my own physics engine / showcase various data science concepts like 3d points clouds, map data etc all in my own ecosystem.

Everything is coded from scratch, trying to implement everything in the way i would do as much as possible, this means that the code is by no means optimal. But this repo is more of my "learning" path on advanced systems engineering. So the code will get refactored to a cleaner code convention, as soon as i start with performance optimizations.

The only Libraries used are GLFW (xlib is terrible), OpenGL, and STB image to load pngs, the rest is 100% selfmade without usage of AI generated code!

---

##  Features

-  Foreward rendering pipeline
-  Physically Based Rendering (PBR) for photorealistic rendering
-  Support for GLTF/GLB models / full scenes including materials / positioning / lighting
-  Custom abstraction, making the engine suitable for easy expansion.
-  Animation system for Camera animations / cutscenes using control points and interpolating between them.
-  Physics engine consising of: Point physics(gravity, drag, conditional forces), Spring constraints, hard length constraints, angular constraints and more to come soon. (this is my current focus of this project, since the graphics part is already pretty feature packed) 
-  Text rendering made from scratch / Overlay rendering with texture atlas support for easy custom font loading.
-  Commandline interface for interacting with elements in the scene in a script-esque way.

---

## Disclaimer

- Basically everything in this repo is made from scratch without AI, therefore some areas are not optimized and are just a best effort to  get the job done in time. Still, even on more complex scenes the engine still averages well above one thousand FPS even on my mediocre hardware.

## More readme coming soon :-)
