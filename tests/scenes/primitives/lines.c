#define SRP_INCLUDE_VEC
#define SRP_INCLUDE_MAT

#include <assert.h>
#include <srp/srp.h>
#include "save.h"

typedef struct Vertex
{
    vec3 position;
    vec3 color;
} Vertex;

typedef struct Uniform
{
	mat4 model;
	mat4 view;
	mat4 projection;
} Uniform;

SRPContext srpContext;

void vertexShader(SRPVertexShaderIn* in, SRPVertexShaderOut* out);
void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out);

int main(int argc, char** argv)
{
    assert(argc >= 2);
    const char* outputPath = argv[1];

    Vertex data[] = {
        // Cube
        // Bottom face (y = -1)
        { .position = VEC3(-1, -1, -1) }, // 0: Front-Left-Bottom
        { .position = VEC3( 1, -1, -1) }, // 1: Front-Right-Bottom
        { .position = VEC3( 1, -1,  1) }, // 2: Back-Right-Bottom
        { .position = VEC3(-1, -1,  1) }, // 3: Back-Left-Bottom

        // Top face (y = 1)
        { .position = VEC3(-1,  1, -1) }, // 4: Front-Left-Top
        { .position = VEC3( 1,  1, -1) }, // 5: Front-Right-Top
        { .position = VEC3( 1,  1,  1) }, // 6: Back-Right-Top
        { .position = VEC3(-1,  1,  1) }, // 7: Back-Left-Top
    };

    uint8_t indices[] = {
        0, 1,  1, 2,  2, 3,  3, 0,  // Bottom square
        4, 5,  5, 6,  6, 7,  7, 4,  // Top square
        0, 4,  1, 5,  2, 6,  3, 7   // Vertical pillars connecting them
    };

    Uniform uniform = {
        .model = mat4ConstructTRS(0, 0,  0,   0, 0.3, 0,   0.5, 0.5, 0.5),
		.view = mat4ConstructView(0, 0, -2,   0, 0,   0,   1,   1,   1),
		.projection = mat4ConstructPerspectiveProjection(-1, 1, -1, 1, 1, 10)
    };

    SRPShaderProgram shaderProgram = {
        .uniform = (SRPUniform*) &uniform,
        .vs = &(SRPVertexShader) {
            .shader = vertexShader,
            .nVaryings = 0,
			.varyingsInfo = NULL,
            .varyingsSize = 0
        },
        .fs = &(SRPFragmentShader) {
            .shader = fragmentShader,
            .mayOverwriteDepth = false
        }
    };

    srpNewContext(&srpContext);
    SRPFramebuffer* fb = srpNewFramebuffer(512, 512);

    SRPVertexBuffer* vb = srpNewVertexBuffer();
    srpVertexBufferCopyData(vb, sizeof(Vertex), sizeof(data), data);
    SRPIndexBuffer* ib = srpNewIndexBuffer();
    srpIndexBufferCopyData(ib, SRP_UINT8, sizeof(indices), indices);

    srpFramebufferClear(fb);
    srpDrawIndexBuffer(ib, vb, fb, &shaderProgram, SRP_PRIM_LINES, 0, 24);

    int ok = saveFramebufferToImage(fb, outputPath);

    srpFreeVertexBuffer(vb);
    srpFreeFramebuffer(fb);

    return ok ? 0 : 1;
}

void vertexShader(SRPVertexShaderIn* in, SRPVertexShaderOut* out)
{
	Vertex* pVertex = (Vertex*) in->vertex;
	Uniform* pUniform = (Uniform*) in->uniform;

	vec3* inPosition = &pVertex->position;
	vec4* outPosition = (vec4*) out->clipPosition;
    *outPosition = VEC4_FROM_VEC3(*inPosition, 1.);
	*outPosition = mat4MultiplyVec4(&pUniform->model, *outPosition);
	*outPosition = mat4MultiplyVec4(&pUniform->view, *outPosition);
	*outPosition = mat4MultiplyVec4(&pUniform->projection, *outPosition);
}

void fragmentShader(SRPFragmentShaderIn* in, SRPFragmentShaderOut* out)
{
    vec4* color = (vec4*) out->color;
    *color = VEC4(1, 1, 1, 1);
}
