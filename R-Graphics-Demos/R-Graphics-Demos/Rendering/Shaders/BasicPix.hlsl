struct PixelIn
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

float4 main(PixelIn input) : SV_TARGET
{
    return float4(input.uv, 0, 1);
}