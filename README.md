# srp

A **s**oftware **r**endering **p**ipeline that features:
- Pixel-perfect rasterization of all main primitive types (triangles, lines, points)
- Fully programmable vertex and fragment shaders (+uniforms)
- Sutherland-Hodgman triangle clipping & Liang-Barsky line clipping
- Perspective-correct attribute interpolation
- Texture mapping
- Post-VS vertex caching
- A small math library to use in shader programming

The only dependency is [`stb_image`](https://github.com/nothings/stb/blob/master/stb_image.h).

Read the documentation for `master` branch [here](https://kitrofimov.github.io/srp/), or build the documentation yourself (see [Building](#building))

## Building

```bash
git clone https://www.github.com/kitrofimov/srp
mkdir srp/build
cd srp/build
cmake .. -D CMAKE_BUILD_TYPE=Release
make
cd bin
```

You can also build the examples with `-D BUILD_EXAMPLES=1` and the documentation with `-D BUILD_DOCS=1` arguments passed to `cmake`. Building the documentation requires having [`dot`](https://en.wikipedia.org/wiki/Graphviz) binary in `PATH`.

## Similar/related projects
- https://github.com/rswinkle/PortableGL
- https://github.com/NotCamelCase/SoftLit
- https://github.com/nikolausrauch/software-rasterizer
- https://github.com/niepp/srpbr

## References
- General Computer Graphics concepts:
    - https://www.scratchapixel.com/
    - https://learnopengl.com/
- Triangle rasteization:
    - https://www.youtube.com/watch?v=k5wtuKWmV48
    - https://dl.acm.org/doi/pdf/10.1145/54852.378457
    - https://acta.uni-obuda.hu/Mileff_Nehez_Dudra_63.pdf
    - https://www.montis.pmf.ac.me/allissues/47/Mathematica-Montisnigri-47-13.pdf
- Perspective-correct interpolation:
    - https://www.comp.nus.edu.sg/%7Elowkl/publications/lowk_persp_interp_techrep.pdf
    - https://www.youtube.com/watch?v=F5X6S35SW2s

## TODO
- [x] Add interpolation with perspective correction
- [x] Split the construction and rasterization of triangles in the pipeline
- [x] Fix rasterization rules (see the gaps between triangles in `03_spinning_textured_cube` example)
- [x] Fix unnecessary allocations in `assembleTriangles()`
- [x] Get rid of dynamic memory allocation in draw calls
    - [x] `malloc`s
    - [x] VLA inside `rasterizeTriangle()`
- [x] Refactor / rewrite initial barycentric calculations
- [x] Fix overly late backface culling
- [x] Make CW/CCW vertex order configurable
- [x] Investigate the behaviour of degenerate triangles (area = 0)
- [x] Refactor / rewrite interpolation logic
    - Perspective-correct interpolation doesn't seem to be right, especially when culling the front faces of the cube in example 03
    - However, both before and after commit f10b4163b43bd3842adc1a651a2d5bbc54ae099b produce pixel-identical results
    - [x] Is perspective-correct interpolation for triangles wrong? Revisit the formulas & math. Using strange `1 / Zndc` instead of `1 / Wclip`?
    - Fixed in b810e73ce6739ff32add7b7cba0ac84704ed94d6
- [x] Check for bottlenecks
- [x] Implement other primitives (lines, points, lines/triangles strip/adjacency etc.)
    - https://wikis.khronos.org/opengl/Primitive
    - [x] Triangle
    - [x] Triangle strip
    - [x] Triangle fan
    - [x] Points
    - [x] Line
    - [x] Line strip
    - [x] Line loop
- [x] Restructure the code (Are the relationships between files clear? Is there any multiple-responsibilities?)
- [x] Clipping
    - [x] Triangles (Sutherland-Hodgman)
    - [x] Lines (Liang-Barsky)
    - [x] Points
- [ ] Debug stutters (e.g. draw cube in line mode, stutters every 1.5-2 seconds)
- [ ] Add an example with `.obj` model loading
- [ ] Add wireframe rendering of triangles (do OpenGL and Vulkan have this?)
- [ ] Should this project use `double`s or `float`s?
- [ ] Check for bottlenecks & optimize
- [ ] Update the documentation
- [ ] Image-based testing framework
- [ ] Implement single-threaded binning and tile system
- [ ] Scale to multiple threads

- [ ] Implement interpolation for types other than `double` (shouldn't it just be `double` and `float`?)
- [ ] Add multisampling
- [ ] Advanced texture techniques:
    - [ ] (Bi)linear filtering
    - [ ] Mipmapping
    - [ ] Anisotropic filtering
    - [ ] Transparent textures?
