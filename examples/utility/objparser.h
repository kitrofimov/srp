#pragma once

#include <srp/vec.h>
#include <stddef.h>

typedef struct OBJVertex {
    vec3d position;
    vec2d uv;
    vec3d normal;
} OBJVertex;

typedef struct OBJMesh {
    OBJVertex* vertices;
    size_t vertexCount;
    uint32_t* indices;
    size_t indexCount;
} OBJMesh;

bool loadOBJMesh(const char* path, OBJMesh* mesh);
void freeOBJMesh(OBJMesh* mesh);
