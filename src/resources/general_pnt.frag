#version 460 core

in vec3 posW;
in vec3 normW;
in vec2 texCoord;

uniform vec3 gDiffuseColor;
uniform vec3 gCamPos;
uniform sampler2D gDiffuseMap;

layout(location = 0) out vec4 outputColor;

void main() {
    vec3 normWFixed = normalize(normW);
    vec3 toCamera = normalize(gCamPos - posW);

    float lightValue = max(dot(toCamera, normWFixed), 0.0f);
    lightValue = (lightValue * 0.7f) + 0.3f;

    vec3 mapColor = texture(gDiffuseMap, texCoord).rgb;
    vec4 diffuseColor = vec4(mapColor * gDiffuseColor * lightValue, 1.0f);

    outputColor = diffuseColor;
}
