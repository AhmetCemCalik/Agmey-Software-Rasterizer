<img src="https://github.com/user-attachments/assets/fb09cfdb-7bc8-42ec-8fa8-7cc25a6dbdec" alt="Gameplay Screenshot 1" height="500">

A lightweight, purely CPU-driven 3D software rasterizer built from scratch in C++ and SDL2. This engine bypasses modern hardware acceleration and graphics APIs (like OpenGL or Vulkan) to replicate the authentic rendering architecture of mid-90s games like *DOOM* and *Duke Nukem 3D*. The entire core geometry pipeline runs in approximately 1,900 lines of code with texture handling included.

## Core Implementation
* **Architecture:** Portal-based sector rendering (not raycasting). Walls are rendered via mathematical Vertex Projection, allowing for non-orthogonal geometry and varying floor/ceiling heights.
* **Floor Casting:** Floors and ceilings are drawn using horizontal Inverse Projection with bitwise math optimizations to eliminate costly floating-point divisions and modulo operators.
* **Lighting & Fog:** Features dynamic, sector-based lighting (`0.0` to `1.0` multipliers) via direct 32-bit channel manipulation, alongside depth-based distance fogging.
* **Performance:** Fixed-point logic and cache-friendly data structures keep frame rates high despite relying strictly on the CPU for all geometry clipping and pixel drawing.

## Properties
The map and textures are completely customizable through the assets folder. The engine will handle loading the map and the textures as long as the manifests are configured correctly. Currently, correct manifest implementation is not documented, altough i believe it is easily inferrable through the comments provided in text files.

## Setup & Build Guide

### Prerequisites
* A C++20 compatible compiler
* [SDL2](https://libsdl.org/) and [SDL2_image](https://github.com/libsdl-org/SDL_image)
* `make` or `CMake`

### Compiling
If building via Makefile, ensure your optimization flags are set (e.g., `-O3`) to allow the CPU to handle the inverse projection math efficiently. Refer to the existing Makefile in the project repo.

```bash
# Clone the repository
git clone [https://github.com/AhmetCemCalik/Agmey-Software-Rasterizer.git](https://github.com/AhmetCemCalik/Agmey-Software-Rasterizer.git)
cd portal-engine

# Build the project
make

# Run the engine
./main-doom
```

### Remarks

This repository is built inside Apple M Series ARM64 architecture and filesystems are configured according to the said architecture. I have not tested compilations in Windows or any other OS, but since the project is entirely SDL and CPP, an executable should be achievable with minor configurations.

The textures are free assets found in the internet, the original source can be tracked through [https://stickyteethgames.itch.io/rust-textures](https://stickyteethgames.itch.io/rust-textures) and is licensed under CC0.
