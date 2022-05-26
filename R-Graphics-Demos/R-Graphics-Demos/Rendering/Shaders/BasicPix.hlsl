
cbuffer Thread : register(b1)
{
    float3 col;
};

struct PixelIn
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

float4 main(PixelIn input) : SV_TARGET
{
    return float4(col, 1);
}