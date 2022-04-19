//
// Atmospheric scattering vertex shader
//
// Author: Sean O'Neil
//
// Copyright (c) 2004 Sean O'Neil
//

#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 fWorldPosition;
out vec3 fPosition;
out mat3 modelMat3;
out vec4 fragPosLightSpace;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

uniform float zCoef; // for log z-buffer (2.0 / log2(farPlane + 1.0)) [логарифмический z-буфер]

void main() {
    fWorldPosition = vec3(model * vec4(aPos, 1.0));
    fPosition = aPos;
    modelMat3 = mat3(model);
    fragPosLightSpace = lightSpaceMatrix * vec4(fWorldPosition, 1.0);

    gl_Position = projection * view * vec4(fWorldPosition, 1);

    /// Log z-buffer [логарифмический z-буфер]
    gl_Position.z = log2(max(1e-6, gl_Position.w + 1.0)) * zCoef - 1.0;
    gl_Position.z *= gl_Position.w;
}