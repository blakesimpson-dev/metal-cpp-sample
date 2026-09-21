# metal-cpp-sample

A learning project in C++20 and Apple's Metal API (metal-cpp). A small renderer
that loads a glTF model and shades it with a perspective camera, depth testing, 
PBR inspired lighting and procedural bump mapping.

The scope is intentionally small, one model and no textures, so that every part 
can be understood and explained. The trajectory of my learning as well as the 
progression from a single tri to the final scene, is tracked in commit history.

| glTF scene (`--scene=gltf`) | Minimal scene (`--scene=minimal`) |
|:---:|:---:|
| ![Exalted Orb render](docs/orb.gif) | ![Minimal triangle](docs/minimal.png) |

## Features

- glTF 2.0 loading with fastgltf (mesh, normals, material factors)
- Perspective camera framed from the model's bounding sphere
- Depth buffer, Lambert diffuse, Blinn-Phong specular
- PBR inspired materials with Fresnel and procedural environment reflection
- Procedural noise bump mapping (surface gradient method, no UVs)
- 4x MSAA, switchable with `--msaa`
- Frame-delta animation

## Build and run

Requires macOS and Xcode (with the Metal toolchain) and CMake 3.26+. Built and
tested with Xcode 27.0 on macOS 27.0 (Apple silicon), using CMake 4.4.3 and
Ninja.

```bash
git clone --recursive \
  https://github.com/blakesimpson-dev/metal-cpp-sample.git
cd metal-cpp-sample
cmake -S . -B build -G Ninja
cmake --build build
./build/metal-cpp-sample [--scene=gltf|minimal] [--msaa=on|off]
```

| Option     | Values                  | Default |
|------------|-------------------------|---------|
| `--scene=` | `gltf`, `minimal`       | `gltf`  |
| `--msaa=`  | `on` (4 samples), `off` | `on`    |

## Layout

| Path            | Purpose                                                  |
|-----------------|----------------------------------------------------------|
| `src/app`       | Application and view delegates                           |
| `src/renderer`  | Metal renderer, camera, math and Metal helpers           |
| `src/scenes`    | `Scene` interface, glTF loader, glTF and minimal scenes  |
| `src/shaders`   | Metal shaders and C++/shader shared types                |
| `src/platform`  | Executable path lookup, metal-cpp ownership helpers      |
| `assets`        | glTF model (Exalted Orb, see Credits)                    |
| `docs`          | README images                                            |
| `external`      | Dependencies (see below)                                 |

## Dependencies

| Dependency           | Version                          | Licence    |
|----------------------|----------------------------------|------------|
| metal-cpp            | macOS27_iOS27 release            | Apache-2.0 |
| metal-cpp-extensions | from Apple's LearnMetalCPP       | Apache-2.0 |
| fastgltf             | v0.9.0                           | MIT        |
| simdjson             | v3.12.3                          | Apache-2.0 |

## Credits

This work is based on
["Exalted Orb"](https://sketchfab.com/3d-models/exalted-orb-8729a148401b4cda8143f61c4c56c3a9)
by [justingulenchyn](https://sketchfab.com/justingulenchyn), licensed under
[CC-BY-4.0](http://creativecommons.org/licenses/by/4.0/).

The application code is my own (MIT, see [LICENSE](LICENSE)); an LLM was used
for explanations, code review and tooling configuration.

## References

- [Apple: metal-cpp](https://developer.apple.com/metal/cpp/) and the
  ["Learn Metal with C++" samples](https://developer.apple.com/metal/LearnMetalCPP.zip)
- [fastgltf](https://github.com/spnda/fastgltf) documentation and headers
- [glTF 2.0 specification](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html)
- LearnOpenGL:
  [Camera](https://learnopengl.com/Getting-started/Camera),
  [Basic Lighting](https://learnopengl.com/Lighting/Basic-Lighting),
  [Advanced Lighting](https://learnopengl.com/Advanced-Lighting/Advanced-Lighting)
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [learncpp.com](https://www.learncpp.com/)
- [Bump Mapping Unparametrized Surfaces on the GPU by Morten S. Mikkelsen](https://mmikk.github.io/papers3d/mm_sfgrad_bump.pdf)
- [three.js `bumpmap_pars_fragment.glsl.js`](https://github.com/mrdoob/three.js/blob/dev/src/renderers/shaders/ShaderChunk/bumpmap_pars_fragment.glsl.js)
- [The Book of Shaders: Noise](https://thebookofshaders.com/11/)
