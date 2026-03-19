#version 460

layout(location = 0) in vec2 vUV;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D uTexture;
layout(binding = 1) uniform sampler2D accumulatorTexture;

layout(std140, binding = 2) uniform PostProcess {
    vec4 mergeOperations;
    vec4 parameters;
} postProcess;

void main() {
    
    int mergeOp = int(postProcess.mergeOperations.x);
    vec4 accColor = texture(accumulatorTexture, vUV);
    vec4 srcColor = texture(uTexture, vUV);
    vec4 texColor = vec4(0.0);

    // alpha blend = 0, add = 1, multiply = 2
    if(mergeOp == 0) {
       texColor.rgb = mix(accColor.rgb, srcColor.rgb, srcColor.a);
       texColor.a = 1.0 - (1.0 - accColor.a) * (1.0 - srcColor.a);
    } else if(mergeOp == 1) {
        texColor.rgb = accColor.rgb + srcColor.rgb;
        texColor.a = clamp(accColor.a + srcColor.a, 0.0, 1.0);
    } else if(mergeOp == 2) {
        texColor.rgb = accColor.rgb * srcColor.rgb;
        texColor.a = accColor.a * srcColor.a;
    }

    outColor = vec4(texColor.rgb, texColor.a);
}