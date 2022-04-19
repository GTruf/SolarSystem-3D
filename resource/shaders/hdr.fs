#version 460 core

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform bool hdr;
uniform float gamma;
uniform float exposure;

out vec4 fragColor;

void main() {
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;

    if(hdr) {
        vec3 result = 1.0 - exp(-hdrColor * exposure);

        // Also gamma correct while we're at it
        result = pow(result, vec3(gamma));
        fragColor = vec4(result, 1.0);
    }
    else {
        vec3 result = pow(hdrColor, vec3(1.0 / gamma));
        fragColor = vec4(result, 1.0);
    }
}