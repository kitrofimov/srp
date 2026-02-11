# srp

A **s**oftware **r**endering **p**ipeline using only CPU computation that features:
- Fully programmable vertex and fragment shaders
- A small math library with vector and matrix structures to use in shader programming
- Texture mapping
- Shader uniforms
- Affine attribute interpolation

Highly inspired by OpenGL API and made mostly for learning purposes, this is my first serious project in C (WIP at the moment) and my first project in the field of computer graphics. The only dependency is [`stb_image`](https://github.com/nothings/stb/blob/master/stb_image.h).

You probably *do not* want to use this in production code! If you need to, see [this awesome project](https://github.com/rswinkle/PortableGL).

Read the documentation for `master` branch [here](https://kitrofimov.github.io/srp/), or build the documentation yourself (see [Building](#building))

## Goals of this project
- Create a general-purpose rendering library (primary use case: CPU-only devices)
- Apply the fundamentals of software design patterns to a real-world case
- Learn basics of computer graphics
- Learn C programming language thoroughly & acquire experience working with it

## What I learned from this project so far (a section for my portfolio)
- Advanced C programming techniques, library design
- Importance of having a plan & an end goal (this project started as something completely different from what it is now)
- Git workflow (+ the importance of atomic commits and meaningful commit names)
- Premature optimization *really* is the root of all evil
- Importance of writing clean code & adding comments

## Building

```bash
git clone https://www.github.com/fahlerile/srp
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

And a lot more... Here I mentioned only those projects on which I looked at as a reference while making this one. If you need more similar projects, see [this](https://github.com/topics/software-rendering) and [this](https://github.com/topics/software-rasterizer).

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

And many, many more, all of which I will not find anymore...

## TODO
- [x] Add interpolation with perspective correction
- [x] Split the construction and rasterization of triangles in the pipeline
- [x] Fix rasterization rules (see the gaps between triangles in `03_spinning_textured_cube` example)
- [x] Fix unnecessary allocations in `assembleTriangles()`
- [x] Get rid of `malloc`s in draw calls
    - [ ] VLA inside `rasterizeTriangle()` - is it bad? should I get rid of it too?
- [x] Refactor / rewrite initial barycentric calculations
- [ ] Investigate the behaviour of degenerate triangles (area = 0)
- [ ] Fix overly late backface culling
- [ ] Make CW/CCW vertex order configurable (check: is it configurable in OpenGL/Vulkan?)
- [ ] Refactor / rewrite interpolation logic
- [ ] Check for bottlenecks
- [ ] Implement other primitives (lines, points, lines/triangles strip/adjacency etc.)
- [ ] Image-based testing framework?
- [ ] Implement single-threaded binning and tile system
- [ ] Scale to multiple threads

- [ ] Implement interpolation for types other than `double` (shouldn't it just be `double` and `float`?)
- [ ] Add multisampling
- [ ] Advanced texture techniques:
    - [ ] (Bi)linear filtering
    - [ ] Mipmapping
    - [ ] Anisotropic filtering
    - [ ] Transparent textures?
