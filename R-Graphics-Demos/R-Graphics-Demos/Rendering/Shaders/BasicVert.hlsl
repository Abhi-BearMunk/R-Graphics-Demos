cbuffer Renderable : register(b0)
{
    float4x4 mat;
};

struct VertexIn
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct VertexOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

VertexOut main(VertexIn vert)
{
    VertexOut vout;
    vout.pos = mul(vert.pos, mat);
    vout.uv = vert.uv;
    return vout;
}