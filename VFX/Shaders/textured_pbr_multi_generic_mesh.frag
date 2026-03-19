#version 460

#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec4 vAlbedo;
layout(location = 3) in float vMetallic;
layout(location = 4) in float vRoughness;
layout(location = 5) in flat uint vTextureIndex;
layout(location = 7) in flat uint vMaskTextureIndex;

layout(location = 0) out vec4 outColor;

layout(binding = 6) uniform sampler2D textures[];

float luminosity(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

void main()
{
    vec3 brightness = vec3(0.12, 0.12, 0.12);
    vec4 texColor = texture(textures[nonuniformEXT(vTextureIndex)], vUV) * vAlbedo;
    vec4 maskColor = vec4(1.0);
    if(vMaskTextureIndex >= 0 && vMaskTextureIndex < 3000) {
        maskColor = texture(textures[nonuniformEXT(vMaskTextureIndex)], vUV);
    }
    float maskAlpha = luminosity(maskColor.rgb);
    texColor = vec4(texColor.rgb + brightness.rgb, maskAlpha);
    outColor = texColor;
}
  