#version 330 core

#extension GL_ARB_shading_language_420pack: enable

const vec2 triVertices[3] = { vec2(-1, -1), vec2(3, -1), vec2(-1, 3) };

out vec2 UV;

void main()
{
    gl_Position = vec4(triVertices[gl_VertexID], 0.0, 1.0);
    UV = vec2(triVertices[gl_VertexID].x, triVertices[gl_VertexID].y);
}
