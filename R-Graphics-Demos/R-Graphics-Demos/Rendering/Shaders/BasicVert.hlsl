cbuffer Renderable : register(b0)
{
    float4x4 mat;
};

struct VertexIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
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
    vout.pos = mul(float4(vert.pos, 1.), mat);
    vout.uv = vert.uv;
    return vout;
}