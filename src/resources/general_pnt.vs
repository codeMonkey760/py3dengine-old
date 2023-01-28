#version 460 core

layout(location = 0) in vec3 posL;
layout(location = 1) in vec3 normL;
layout(location = 2) in vec2 texC;

uniform mat4 gWMtx;
uniform mat4 gWITMtx;
uniform mat4 gWVPMtx;

out vec3 posW;
out vec3 normW;
out vec2 texCoord;

void main() {
    posW = (vec4(posL, 1.0f) * gWMtx).xyz;
    normW = (vec4(normL, 0.0f) * gWITMtx).xyz;
    texCoord = texC;

    gl_Position = (vec4(posL, 1.0) * gWVPMtx);
}
