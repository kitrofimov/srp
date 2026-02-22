#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "objparser.h"

bool loadOBJMesh(const char* path, OBJMesh* mesh)
{
    FILE* file = fopen(path, "r");
    if (!file) return false;

    // Temporary storage for positions, uvs, normals
    vec3d* tempPositions = malloc(sizeof(vec3d) * 65536);
    vec2d* tempUVs       = malloc(sizeof(vec2d) * 65536);
    vec3d* tempNormals   = malloc(sizeof(vec3d) * 65536);
    size_t posCount = 0, uvCount = 0, normalCount = 0;

    // Allocate mesh arrays (will realloc if needed)
    mesh->vertices = malloc(sizeof(OBJVertex) * 65536);
    mesh->indices  = malloc(sizeof(uint32_t) * 65536);
    mesh->vertexCount = 0;
    mesh->indexCount  = 0;

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == 'v' && line[1] == ' ' && posCount < 65536)  // Vertex position
        {
            double x, y, z;
            sscanf(line, "v %lf %lf %lf", &x, &y, &z);
            tempPositions[posCount] = (vec3d) {x, y, z};
            posCount++;
        }
        else if (line[0] == 'v' && line[1] == 't' && uvCount < 65536)  // Texture coord
        {
            double u, v;
            sscanf(line, "vt %lf %lf", &u, &v);
            tempUVs[uvCount] = (vec2d) {u, v};
            uvCount++;
        }
        else if (line[0] == 'v' && line[1] == 'n' && normalCount < 65536) // Normals
        {
            double nx, ny, nz;
            sscanf(line, "vn %lf %lf %lf", &nx, &ny, &nz);
            tempNormals[normalCount] = (vec3d) {nx, ny, nz};
            normalCount++;
        }
        else if (line[0] == 'f')  // Face
        {
            int vi[3], ti[3], ni[3];  // Vertex, texture, normal indices
            int scanned = sscanf(
                line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vi[0], &ti[0], &ni[0],
                &vi[1], &ti[1], &ni[1],
                &vi[2], &ti[2], &ni[2]
            );
            if (scanned == 9)
                for (int i = 0; i < 3; i++)
                {
                    OBJVertex v;
                    v.position = tempPositions[vi[i] - 1];
                    v.uv       = tempUVs[ti[i] - 1];
                    v.normal   = tempNormals[ni[i] - 1];

                    mesh->vertices[mesh->vertexCount] = v;
                    mesh->indices[mesh->indexCount]   = mesh->vertexCount;
                    mesh->vertexCount++;
                    mesh->indexCount++;
                }
            else
                fprintf(stderr, "Unsupported face format: %s\n", line);
        }
    }

    free(tempPositions);
    free(tempUVs);
    free(tempNormals);
    fclose(file);

    return true;
}

void freeOBJMesh(OBJMesh* mesh)
{
    assert(mesh != NULL);
    free(mesh->vertices);
    free(mesh->indices);
}
