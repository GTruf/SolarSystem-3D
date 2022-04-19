#version 460 core

layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 particleCenterWorldSpace;
uniform bool is3D;

void main() {
    if (is3D) {
        vec3 vertexPosition_worldspace = particleCenterWorldSpace;

        // Get the screen-space position of the particle's center
        gl_Position = projection * view * vec4(vertexPosition_worldspace, 1.0f);
        // Here we have to do the perspective division ourselves.
        gl_Position /= gl_Position.w;
        // Move the vertex in directly screen space. No need for CameraUp/Right_worldspace here.
        gl_Position.xy += vertex.xy * vec2(0.005, 0.01);
    }
    else {
        gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    }

    TexCoords = vertex.zw;
}