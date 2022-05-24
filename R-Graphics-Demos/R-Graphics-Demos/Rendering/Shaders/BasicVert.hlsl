cbuffer Renderable : register(b0)
{
    float2 offset;
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
    vout.pos = vert.pos + float4(offset, 0, 0);
    vout.uv = vert.uv;
    return vout;
}