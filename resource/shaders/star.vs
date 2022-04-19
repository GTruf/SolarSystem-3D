#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 fPosition;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform float zCoef; // for log z-buffer (2.0 / log2(farPlane + 1.0)) [логарифмический z-буфер]

void main() {
    fPosition = aPos;
    gl_Position = projection * view * model * vec4(aPos, 1.0f);

    /// Log z-buffer [логарифмический z-буфер]
    gl_Position.z = log2(max(1e-6, gl_Position.w + 1.0)) * zCoef - 1.0;
    gl_Position.z *= gl_Position.w;
}