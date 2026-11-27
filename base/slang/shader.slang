/* Copyright (c) 2025-2026, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

struct VSInput {
    float2 Pos;
    float3 Color;
	//float2 UV;
};

Sampler2D textures[];

struct ShaderData {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

struct VSOutput {
    float4 Pos : SV_POSITION;
    float3 Color;
    //float2 UV;
    uint32_t InstanceIndex;
};

[shader("vertex")]
VSOutput main(VSInput input, uniform ShaderData *shaderData, uint instanceIndex : SV_VulkanInstanceID) {
    VSOutput output;
    output.Color = input.Color;
    output.Pos = float4(input.Pos.x, input.Pos.y, 0, 1);
    output.InstanceIndex = instanceIndex;
    return output;
}

[shader("fragment")]
float4 main(VSOutput input) {
    return float4(input.Color.rgb, 1.0);
}
