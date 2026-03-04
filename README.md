# srp

A **s**oftware **r**endering **p**ipeline that features:
- Pixel-perfect rasterization of all main primitive types (triangles, lines, points)
- Fully programmable vertex and fragment shaders (+uniforms)
- Sutherland-Hodgman triangle clipping & Liang-Barsky line clipping
- Perspective-correct, affine, and flat attribute interpolation
- Texture mapping
- Post-VS vertex caching
- A small math library to use in shader programming
- Image-based testing framework

The only dependency is [`stb_image`](https://github.com/nothings/stb/blob/master/stb_image.h).

If you want to use this library in your own project, you only need headers from `include/srp` directory.

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

`BUILD_EXAMPLES`, `BUILD_DOCS`, and `BUILD_TESTS` options are also available (i.e. `-D BUILD_EXAMPLES=1` passed to `cmake`). Building the documentation requires having [`dot`](https://en.wikipedia.org/wiki/Graphviz) binary in `PATH`; building the tests requires `numpy` and `pillow` Python modules. After building, the examples, docs, and tests will appear in `build/examples`, `build/docs`, and `build/tests`.

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
- [x] Fix rasterization rules
- [x] Get rid of dynamic memory allocation in the hot path (`malloc` and VLA)
- [x] Make CW/CCW vertex order configurable
- [x] Implement other primitives (lines, points, lines/triangles strip/adjacency etc.)
- [x] Project refactoring
- [x] Clipping
- [x] Debug stutters (e.g. draw cube in line mode, stutters every 1.5-2 seconds)
- [x] Add an example with `.obj` model loading
- [x] Add wireframe rendering of triangles (polygon rendering mode)
- [x] Use `float`s everywhere (instead of `double`s)
- [x] Check for bottlenecks & optimize
- [x] Update the documentation
- [x] Image-based testing framework
- [x] Flat interpolation, per-varying perspective / affine / flat setting
- [ ] Fix #30
- [ ] Phong shading example
- [ ] Bilinear filtering
- [ ] Mipmapping
- [ ] Depth test options
- [ ] Scissor test
- [ ] Stencil test
- [ ] Blending
- [ ] sRGB
- [ ] MSAA (multisampling)
- [ ] Single-threaded binning and tile system
- [ ] Scale to multiple threads
