#include "pch.h"
#include "BasePass.h"

R::Rendering::BasePass::BasePass(RenderContext* globalContext, Job::JobSystem* jobSystem)
	:m_pRenderContext(globalContext), m_pJobSystem(jobSystem)
{
    for (int i = 0; i < ECS::MAX_ENTITIES_PER_ARCHETYPE; i++)
    {
        m_jobDescs[i].jobFunc = &BasePass::JobFunc;
        m_jobDescs[i].param = &m_jobDatas[i];
        m_jobDescs[i].pCounter = &m_jobCounter;
        m_jobDatas[i].basePass = this;
    }
}

R::Rendering::BasePass::~BasePass()
{
    WaitForCompletion();
    delete[] m_jobDescs;
    delete[] m_jobDatas;
}

void R::Rendering::BasePass::Init(ID3D12GraphicsCommandList* cmdList)
{
    SetupRSAndPSO();
    SetupVertexBuffer(cmdList);
}

void R::Rendering::BasePass::Update(FrameResource* frameResource, const CD3DX12_CPU_DESCRIPTOR_HANDLE* rtvHandle)
{
    for (std::uint32_t i = 0; i < m_pJobSystem->GetNumWorkers(); i++)
    {
        auto commandList = m_pRenderContext->GetThreadContext(i)->GetCommandList();
        commandList->SetPipelineState(m_pipelineState.Get());
        commandList->SetGraphicsRootSignature(m_rootSignature.Get());
        commandList->OMSetRenderTargets(1, rtvHandle, FALSE, nullptr);

        // Record commands.
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        commandList->IASetIndexBuffer(&m_indexBufferView);
    }

    m_currentFrameResource = frameResource;

    std::uint32_t updateBatchSize = std::max(100u, frameResource->GetCount() / 64);
    std::uint32_t count = 0;
    for (std::uint32_t i = 0; i < frameResource->GetCount(); i += updateBatchSize)
    {
        m_jobDatas[count].startIndex = i;
        m_jobDatas[count].batchSize = std::min(updateBatchSize, frameResource->GetCount() - i);
        count++;
    }
    assert(m_jobCounter.counter == 0);
    m_jobCounter.counter = count;
    m_pJobSystem->KickJobsWithPriority(m_jobDescs, count);
}

void R::Rendering::BasePass::WaitForCompletion()
{
    m_pJobSystem->WaitForCounter(&m_jobCounter);
}

void R::Rendering::BasePass::SetupRSAndPSO()
{
    // ROOT SIG
    CD3DX12_ROOT_PARAMETER rp[1];
    rp[0].InitAsConstants(SIZE_OF_32(Renderable), 0); // b0

    CD3DX12_ROOT_SIGNATURE_DESC rs(_countof(rp), rp);

    rs.Init(1, rp, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    HRESULT hr = D3D12SerializeRootSignature(&rs, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    if (FAILED(hr))
    {
        R_LOG_ERROR("Failed to initialize root signature for Draw Triange Pass");
        if (error)
        {
            R_LOG_ERROR(reinterpret_cast<const char*>(error->GetBufferPointer()));
        }
    }

    LogErrorIfFailed(m_pRenderContext->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf())));

    // PSO
    auto vertexShaderBlob = R::Utils::ReadData(L"BasicVert.cso");
    auto pixelShaderBlob = R::Utils::ReadData(L"BasicPix.cso");

    static const D3D12_INPUT_ELEMENT_DESC s_inputElementDesc[4] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,  0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,  0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,  0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,  0 }
    };

    auto rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    //rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

    // Describe and create the graphics pipeline state objects (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { s_inputElementDesc, _countof(s_inputElementDesc) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { vertexShaderBlob.data(), vertexShaderBlob.size() };
    psoDesc.PS = { pixelShaderBlob.data(), pixelShaderBlob.size() };
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    //psoDesc.DSVFormat = GetDepthBufferFormat();
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    LogErrorIfFailed(
        m_pRenderContext->GetDevice()->CreateGraphicsPipelineState(&psoDesc,
            IID_PPV_ARGS(m_pipelineState.ReleaseAndGetAddressOf())));
}


//struct Vertex
//{
//    DirectX::XMFLOAT3 position;
//    DirectX::XMFLOAT3 normal;
//    DirectX::XMFLOAT2 uv;
//};

void R::Rendering::BasePass::SetupVertexBuffer(ID3D12GraphicsCommandList* cmdList)
{
    // Create the vertex buffer.
    {
        // Define the geometry for a triangle.
        PrimitivesGenerator::MeshData cubeMesh = PrimitivesGenerator::CreateGeosphere(1, 2);

        CommandCreateBufferFromData(m_pRenderContext->GetDevice(),
            cmdList,
            m_vertexBuffer.ReleaseAndGetAddressOf(),
            m_vertexBufferUploader.ReleaseAndGetAddressOf(),
            cubeMesh.Vertices.data(),
            cubeMesh.Vertices.size() * sizeof(PrimitivesGenerator::Vertex),
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(PrimitivesGenerator::Vertex);
        m_vertexBufferView.SizeInBytes = cubeMesh.Vertices.size() * sizeof(PrimitivesGenerator::Vertex);

        CommandCreateBufferFromData(m_pRenderContext->GetDevice(),
            cmdList,
            m_indexBuffer.ReleaseAndGetAddressOf(),
            m_indexBufferUploader.ReleaseAndGetAddressOf(),
            cubeMesh.GetIndices16().data(),
            cubeMesh.GetIndices16().size() * sizeof(uint16_t),
            D3D12_RESOURCE_STATE_INDEX_BUFFER);

        // Initialize the index buffer view.
        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        m_indexBufferView.SizeInBytes = cubeMesh.GetIndices16().size() * sizeof(uint16_t);
    }
}

void R::Rendering::BasePass::JobFunc(void* param, std::uint32_t tid)
{
    JobData* data = reinterpret_cast<JobData*>(param);
    Renderable* renderable;
    auto commandList = data->basePass->m_pRenderContext->GetThreadContext(tid)->GetCommandList();

    //auto inited = (m_threadInitFlag >> tid) & 1u;
    //// If job is starting on this thread
    //if (inited == 0)
    //{
    //    m_threadInitFlag += 1u << tid;
    //    commandList->SetPipelineState(data->basePass->m_pipelineState.Get());
    //    commandList->SetGraphicsRootSignature(data->basePass->m_rootSignature.Get());
    //    commandList->OMSetRenderTargets(1, data->basePass->m_currentRtvHandle, FALSE, nullptr);

    //    // Record commands.
    //    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    //    commandList->IASetVertexBuffers(0, 1, &data->basePass->m_vertexBufferView);
    //}

    for (std::uint32_t k = 0; k < data->batchSize; k++)
    {
        renderable = data->basePass->m_currentFrameResource->GetRenderable(data->startIndex + k);
        commandList->SetGraphicsRoot32BitConstants(0, SIZE_OF_32(Renderable), renderable, 0);
        commandList->DrawIndexedInstanced(960, 1, 0, 0, 0);
    }
}
