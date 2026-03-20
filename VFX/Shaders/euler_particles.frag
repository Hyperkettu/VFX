#version 460

layout(location = 0) in vec2 vUV;
layout(location = 1) in vec4 vColor;

layout(location = 0) out vec4 outColor;

//layout(binding = 1) uniform sampler2D uTextured;

void main() {
  //  vec4 texColor = texture(uTexture, vUV);
    outColor = vColor;
}