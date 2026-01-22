<h1 align="center">
  Penumbra
</h1>

![alien](https://github.com/user-attachments/assets/79021d29-da05-4e00-a0e6-5a68996dd6dc)
**Penumbra is a compact, physically-based path tracing renderer built for fast iteration and rendering experiments.**

## Motivation
After taking the amazing [CS 6620, Rendering with Ray Tracing](https://graphics.cs.utah.edu/courses/cs6620/fall2025/) course by the legendary [Cem Yuksel](https://www.cemyuksel.com/) at the University of Utah, I decided to combine all my learnings into a single renderer after completing the course, hoping to use it for future research and projects.

## Features
- **Rendering**
  - Physically-based path tracing (progressive Monte Carlo global illumination)
  - Multiple importance sampling (MIS)
  - Next-event-estimation (NEE)
  - PBRT v3 scene system (.pbrt), with support for all major 3D file formats (.obj, .fbx ...)
  - ~USD/Hydra 2.0 support~ (wip)

- **Tools**
  - Interactive user interface with in-progress render preview
  - Render export (.jpg, .png, .exr, ...)

- **Shading**
  - Walt Disney Animation Studios' [principled BSSRDF](https://disneyanimation.com/publications/physically-based-shading-at-disney/) (microfacet theory / GGX)
    - Material parameters: albedo, roughness, metalness, refraction, ~other~ (wip)
  - Textures: textured material parameters, normal mapping

- **Lighting**
  - Ideal lights: point, ~spotlight~ (wip), ~directional~ (wip), ~infinite~ (wip)
  - Area lights: sphere, mesh, ~quad~ (wip), ~disk~ (wip)
  - Environment mapping (Simple/HDRI)

- **Post-processing**
  - Gamma correction
  - Tone mapping based on Uncharted 2's ["Filmic" model](https://64.github.io/tonemapping/#uncharted-2)

- **Stereo**
  - Anaglyph stereo 3D (red/cyan)

- **Animation**
  - Python script-based scene generation
  - Live per-frame animated render preview and export system

- **Performance**
  - Tile-based multithreading
  - Morton (Z) ordering
  - Acceleration structures: BLAS (BVH), ~TLAS~ (wip)
  - ~Adaptive sampling~ (wip)
  - ~Headless (CLI) mode~ (wip)

## Gallery
https://github.com/user-attachments/assets/6509d9c4-4483-4d58-88a4-2c83f58304f4

https://github.com/user-attachments/assets/6ad0b839-c02f-41c2-8786-0c1d049dd6b1

<img width="1920" height="1080" alt="delta_teapots" src="https://github.com/user-attachments/assets/642303c6-7cef-4b0b-9ddb-72380b39566e" />
<img width="958" height="568" alt="Cornell" src="https://github.com/user-attachments/assets/b89eaaa0-631a-41ac-b176-e3a22c645a27" />
<img width="1920" height="1080" alt="teapot_roughness" src="https://github.com/user-attachments/assets/4fe710ed-1e38-4ade-a253-0961fbfa52db" />

## Quick start
### Building

**Prerequisites:** CMake 3.5+, C++17 compiler, and [OpenImageIO](https://github.com/AcademySoftwareFoundation/OpenImageIO) (build separately or `brew install openimageio` on macOS).

### Windows
```bash
cd scripts
build.bat
```

Or manually:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### macOS / Linux
```bash
chmod +x scripts/build.sh
./scripts/build.sh
```

Or manually:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Options
- `--debug` – Debug build
- `--no-verify` – Skip library tests
- `--no-optimize` – Disable optimizations
- `--headless` – Build without UI
- `--help` - show command info

Run `build.bat --help` or `./build.sh --help` for all options.

### Dependencies

**All dependencies are installed using GitHub's FetchContent feature if the build scripts are used (except OpenImageIO)**, however for a manual install the following are required:

- [GLM](https://github.com/g-truc/glm) – Math library for graphics
- [GLFW](https://github.com/glfw/glfw) – Window and input management
- [Assimp](https://github.com/assimp/assimp) – 3D model loading
- [TinyBVH](https://github.com/jbikker/tinybvh) – BVH acceleration structure
- [OpenImageIO](https://github.com/OpenImageIO/oiio) – Image I/O (must be built separately)
- [ImGui](https://github.com/ocornut/imgui) – UI framework
- [minipbrt](https://github.com/vilya/minipbrt) – PBRT file format parser

## Contributing
- This is a personal research/learning renderer, but contributions are welcome:
- Bugs + minimal repro scenes are especially helpful
- Prefer small, focused PRs

## Credits / thanks / references
- [Physically Based Rendering:From Theory To Implementation](https://www.pbr-book.org/)
- [Physically-Based Shading at Disney](https://disneyanimation.com/publications/physically-based-shading-at-disney/)
- [Tone Mapping](https://64.github.io/tonemapping/) by [Matt Taylor](https://github.com/64)
- Joe Schutte's [article on Disney BSSRDF implementation](https://schuttejoe.github.io/post/disneybsdf/)
- Thanks to:
  - Professor [Cem Yuksel](https://www.cemyuksel.com/) for the coolest computer graphics course ever
  - My mentor [Emily Vo](https://emily-vo.github.io/) for their advice and direction in making this renderer
  - My mentor [Karl Li](https://www.yiningkarlli.com/) for his guidance and resources to improve this project (and the amazing blog on his renderer [Takua](https://www.yiningkarlli.com/projects/takuarenderer.html)!) 
  - My peers at the University of Utah, especially Kyle Webster, Andrew Tate, Austin Kim and Conner Murray, and everyone at the [Utah Graphics Lab](https://graphics.cs.utah.edu/) for their support and advice

## Known issues / current limitations
- Delta / perfect specular materials need dedicated handling
- Current MIS is correct for non-delta BxDFs; delta-specular transport integration on roadmap
- No importance sampling (luminance CDF) for IBL (image base lighting)
- Metallic + roughness edge cases may converge poorly until delta + microfacet details are finalized
