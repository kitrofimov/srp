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

typedef struct VSOutput
{
    vec3 color;
} VSOutput;

typedef struct Uniform
{
	mat4 model;
	mat4 view;
	mat4 projection;
} Uniform;

SRPContext srpContext;

void vertexShader(SRPvsInput* in, SRPvsOutput* out);
void fragmentShader(SRPfsInput* in, SRPfsOutput* out);

int main(int argc, char** argv)
{
    assert(argc >= 2);
    const char* outputPath = argv[1];

    Vertex data[] = {
        // Cube
        // Bottom face (y = -1)
        { .position = { -1, -1, -1 }, .color = { 1, 1, 1 } }, // 0: Front-Left-Bottom
        { .position = {  1, -1, -1 }, .color = { 1, 1, 1 } }, // 1: Front-Right-Bottom
        { .position = {  1, -1,  1 }, .color = { 1, 1, 1 } }, // 2: Back-Right-Bottom
        { .position = { -1, -1,  1 }, .color = { 1, 1, 1 } }, // 3: Back-Left-Bottom

        // Top face (y = 1)
        { .position = { -1,  1, -1 }, .color = { 1, 1, 1 } }, // 4: Front-Left-Top
        { .position = {  1,  1, -1 }, .color = { 1, 1, 1 } }, // 5: Front-Right-Top
        { .position = {  1,  1,  1 }, .color = { 1, 1, 1 } }, // 6: Back-Right-Top
        { .position = { -1,  1,  1 }, .color = { 1, 1, 1 } }, // 7: Back-Left-Top
    };

    uint8_t indices[] = {
        0, 1,  1, 2,  2, 3,  3, 0,  // Bottom square
        4, 5,  5, 6,  6, 7,  7, 4,  // Top square
        0, 4,  1, 5,  2, 6,  3, 7   // Vertical pillars connecting them
    };

    Uniform uniform = {
        .model = mat4ConstructTRS(0, 0,  0,     0, 0.4, 0,   0.5, 0.5, 0.5),
		.view = mat4ConstructView(0, 0, -1.5,   0, 0,   0,   1,   1,   1),
		.projection = mat4ConstructPerspectiveProjection(-1, 1, -1, 1, 1, 10)
    };

    SRPShaderProgram shaderProgram = {
        .uniform = (SRPUniform*) &uniform,
        .vs = &(SRPVertexShader) {
            .shader = vertexShader,
            .nVaryings = 1,
            .varyingsInfo = (SRPVaryingInfo[]) {
                { .nItems = 3, .type = SRP_FLOAT }
            },
            .varyingsSize = sizeof(VSOutput)
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

void vertexShader(SRPvsInput* in, SRPvsOutput* out)
{
	Vertex* pVertex = (Vertex*) in->vertex;
	Uniform* pUniform = (Uniform*) in->uniform;
	VSOutput* pOutVars = (VSOutput*) out->varyings;

	vec3* inPosition = &pVertex->position;
	vec4* outPosition = (vec4*) out->position;
	*outPosition = (vec4) {
		inPosition->x, inPosition->y, inPosition->z, 1.0
	};
	*outPosition = mat4MultiplyVec4(&pUniform->model, *outPosition);
	*outPosition = mat4MultiplyVec4(&pUniform->view, *outPosition);
	*outPosition = mat4MultiplyVec4(&pUniform->projection, *outPosition);

	pOutVars->color = pVertex->color;
}

void fragmentShader(SRPfsInput* in, SRPfsOutput* out)
{
    VSOutput* i = (VSOutput*) in->varyings;

    vec4* color = (vec4*) out->color;
    color->x = i->color.x;
    color->y = i->color.y;
    color->z = i->color.z;
    color->w = 1.;
}
