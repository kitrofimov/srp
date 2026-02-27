Overview {#mainpage}
========

SRP (stands for <b>s</b>oftware <b>r</b>endering <b>p</b>ipeline) is an OpenGL-like CPU graphics library written in C with minimal dependencies. It was made as a hobby programming project. The source code (and more information) is available on [Github](https://www.github.com/fahlerile/srp).

A nice way to get familiar with API is to read <a href="examples.html">Examples</a> or <a href="group__API__reference.html">API reference</a>. If you are interested in implmentation details, see <a href="group__Internal.html">Internal documentation</a>.

Here is a graph overviewing the pipeline: (red edges - the main path from a draw call to a fragment on the screen, green nodes - objects created by user)

@dot
    graph {
        vertex_data [label="Vertex data"];
        index_data [label="Index data"];
        vertex_cache [label="Vertex cache"];
        primitive_assembly [label="Primitive assembly"];

        VertexBuffer [color=green];
        IndexBuffer [color=green];
        ShaderProgram [color=green];
        VertexShader [color=green];
        FragmentShader [color=green];
        Uniform [color=green];

        VertexBuffer -- vertex_data;
        IndexBuffer -- index_data;
        index_data -- vertex_data;
        vertex_data -- Drawcall;
        index_data -- Drawcall [style=dashed];
        ShaderProgram -- VertexShader,FragmentShader,Uniform;
        FragmentShader,VertexShader -- Uniform;

        edge [color=red];

        Drawcall -- primitive_assembly -- vertex_cache -- VertexShader -- Clipping -- Rasterizer -- Interpolator -- FragmentShader;
    }
@enddot
