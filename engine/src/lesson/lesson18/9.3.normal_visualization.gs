#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

in vec3 vPos[];
in vec3 vNormal[];

uniform mat4 view;
uniform mat4 projection;

const float normalLength = 0.1;

void main()
{
    for (int i = 0; i < 3; ++i)
    {
        vec3 start = vPos[i];
        vec3 end = vPos[i] + normalize(vNormal[i]) * normalLength;

        gl_Position = projection * view * vec4(start, 1.0);
        EmitVertex();
        gl_Position = projection * view * vec4(end, 1.0);
        EmitVertex();
        EndPrimitive();
    }
}
