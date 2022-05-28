
cbuffer Thread : register(b1)
{
    float3 col;
};

struct PixelIn
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

struct Pixel
{
    float4 color : SV_Target;
};

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

Pixel main(PixelIn input) : SV_TARGET
{
    //return float4(col, 1);
    Pixel Out;
    Out.color = txDiffuse.Sample(samLinear, input.uv);
    return Out;
}