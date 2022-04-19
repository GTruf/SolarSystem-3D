#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

uniform vec3 camPos;
uniform vec3 lightPos;

uniform float zCoef; // For log z-buffer (2.0 / log2(farPlane + 1.0))

out vec3 fPosition;
out vec3 normal;
out vec4 fragPosLightSpace;
out vec2 texCoords;
out vec3 fWorldPosition;
out float fLogZ;

out vec3 vEyePos;
out vec3 vLight0Pos;

void main() {
    fWorldPosition = vec3(model * vec4(aPos, 1.0));
    fPosition = vec3(aPos.x, 0.0, aPos.z);

    vEyePos = camPos - fWorldPosition.xyz;
    vLight0Pos = lightPos - fWorldPosition.xyz;

    mat3 normalMatrix = mat3(transpose(inverse(model)));
    normal = normalize(normalMatrix * aNormal);

    fragPosLightSpace = lightSpaceMatrix * model * vec4(aPos, 1.0);
    texCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos.x, 0.0, aPos.z, 1.0);

    // Log z-buffer [логарифмический z-буфер]
    gl_Position.z = log2(max(1e-6, gl_Position.w + 1.0)) * zCoef - 1.0;
    gl_Position.z *= gl_Position.w;

    fLogZ = 1.0 + gl_Position.w;
}