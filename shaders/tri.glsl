#version 330 core

//#extension GL_ARB_shading_language_420pack: enable

uniform mat4 view;
uniform mat4 scale;

const vec2 triVertices[3] = vec2[]( vec2(-1, -1), vec2(3, -1), vec2(-1, 3) );

out vec2 UV;

void main()
{
    gl_Position = scale * view * vec4(triVertices[gl_VertexID], 0.0, 1.0);
    
    UV = vec2(triVertices[gl_VertexID].x, triVertices[gl_VertexID].y);
}
