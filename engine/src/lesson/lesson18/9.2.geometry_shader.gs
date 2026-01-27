#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vPos[];
in vec3 vNormal[];
in vec2 vTexCoord[];

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    float magnitude = 0.2 * sin(time);
    for (int i = 0; i < 3; ++i)
    {
        vec3 offset = vPos[i] + vNormal[i] * magnitude;
        gl_Position = projection * view * model * vec4(offset, 1.0);
        TexCoord = vTexCoord[i];
        EmitVertex();
    }
    EndPrimitive();
}
