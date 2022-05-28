#include "pch.h"
#include "BasePass.h"

R::Rendering::BasePass::BasePass(RenderContext* renderContext, Job::JobSystem* jobSystem)
	:m_pJobSystem(jobSystem), m_jobConstData(renderContext)
{
    for (int i = 0; i < ECS::MAX_ENTITIES_PER_ARCHETYPE; i++)
    {
        m_jobDescs[i].jobFunc = &BasePass::JobFunc;
        m_jobDescs[i].param = &m_jobDatas[i];
        m_jobDescs[i].pCounter = &m_jobCounter;
        m_jobDatas[i].constData = &m_jobConstData;
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

void R::Rendering::BasePass::Update()
{
    auto renderContext = m_jobConstData.renderContext;
    for (std::uint32_t i = 0; i < m_pJobSystem->GetNumWorkers(); i++)
    {
        auto commandList = renderContext->GetThreadContext(i)->GetCommandList();
        commandList->SetPipelineState(m_pipelineState.Get());
        commandList->SetGraphicsRootSignature(m_rootSignature.Get());
        ID3D12DescriptorHeap* pHeaps[] = { renderContext->GetCBVSRVUAVDescriptorHeap() };
        commandList->SetDescriptorHeaps(_countof(pHeaps), pHeaps);
        auto rtvHandle = renderContext->GetCurrentRTVHandle();
        auto dsvHandle = renderContext->GetCurrentDSVHandle();
        commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

        // Record commands.
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        commandList->IASetIndexBuffer(&m_indexBufferView);
    }

    m_jobConstData.renderContext = renderContext;
    auto frameResource = renderContext->GetCurrentFrameResource();

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
    auto renderContext = m_jobConstData.renderContext;
    // ROOT SIG
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(renderContext->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    CD3DX12_DESCRIPTOR_RANGE1 ranges[1]; // Perfomance TIP: Order from most frequent to least frequent.
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);    // frequently changed diffuse + normal textures - using registers t1 and t2.
    //ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);    // 1 frequently changed constant buffer.
    //ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);                                                // 1 infrequently changed shadow texture - starting in register t0.
    //ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);                                            // static samplers.

    CD3DX12_ROOT_PARAMETER1 rootParameters[3];
    rootParameters[0].InitAsConstants(SIZE_OF_32(Renderable), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX); // b0
    rootParameters[1].InitAsConstants(SIZE_OF_32(XMFLOAT3), 1, 0, D3D12_SHADER_VISIBILITY_PIXEL); // b1
    rootParameters[2].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
    //rootParameters[3].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    samplerDesc.MinLOD = 2;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
    if (FAILED(hr))
    {
        R_LOG_ERROR("Failed to initialize root signature for Draw Triange Pass");
        if (error)
        {
            R_LOG_ERROR(reinterpret_cast<const char*>(error->GetBufferPointer()));
        }
    }
    LogErrorIfFailed(renderContext->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    NAME_D3D12_COMPTR(m_rootSignature);

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
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state;
    psoDesc.DSVFormat = renderContext->GetDepthBufferFormat();
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = renderContext->GetRenderTargetFormat();
    psoDesc.SampleDesc.Count = 1;
    LogErrorIfFailed(
        renderContext->GetDevice()->CreateGraphicsPipelineState(&psoDesc,
            IID_PPV_ARGS(m_pipelineState.ReleaseAndGetAddressOf())));
}

void R::Rendering::BasePass::SetupVertexBuffer(ID3D12GraphicsCommandList* cmdList)
{
    auto renderContext = m_jobConstData.renderContext;
    // Create the vertex buffer.
    {
        // Define the geometry for a triangle.
        PrimitivesGenerator::MeshData mesh = PrimitivesGenerator::CreateBox(1, 1, 1, 0);
        UINT vertexBufferSize = static_cast<UINT>(mesh.Vertices.size()) * sizeof(PrimitivesGenerator::Vertex);
        UINT indexBufferSize = static_cast<UINT>(mesh.GetIndices16().size()) * sizeof(uint16_t);

        CommandCreateBufferFromData(renderContext->GetDevice(),
            cmdList,
            m_vertexBuffer.ReleaseAndGetAddressOf(),
            m_vertexBufferUploader.ReleaseAndGetAddressOf(),
            mesh.Vertices.data(),
            vertexBufferSize,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(PrimitivesGenerator::Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;

        CommandCreateBufferFromData(renderContext->GetDevice(),
            cmdList,
            m_indexBuffer.ReleaseAndGetAddressOf(),
            m_indexBufferUploader.ReleaseAndGetAddressOf(),
            mesh.GetIndices16().data(),
            indexBufferSize,
            D3D12_RESOURCE_STATE_INDEX_BUFFER);

        // Initialize the index buffer view.
        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        m_indexBufferView.SizeInBytes = indexBufferSize;
    }
}

XMFLOAT3 colors[] = {
    {230 / 255.f, 25 / 255.f, 75 / 255.f},
    {245 / 255.f, 130 / 255.f, 48 / 255.f},
    {210 / 255.f, 245 / 255.f, 60 / 255.f},
    {70 / 255.f, 240 / 255.f, 240 / 255.f},
    {0 / 255.f, 128 / 255.f, 128 / 255.f},
    {220 / 255.f, 190 / 255.f, 255 / 255.f},
    {128 / 255.f, 128 / 255.f, 0 / 255.f},
    {128 / 255.f, 128 / 255.f, 128 / 255.f}
};

void R::Rendering::BasePass::JobFunc(void* param, std::uint32_t tid)
{
    JobData* data = reinterpret_cast<JobData*>(param);
    Renderable* renderable;
    auto renderContext = data->constData->renderContext;
    auto commandList = renderContext->GetThreadContext(tid)->GetCommandList();
    auto frameResource = renderContext->GetCurrentFrameResource();
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
        renderable = frameResource->GetRenderable(data->startIndex + k);
        commandList->SetGraphicsRoot32BitConstants(0, SIZE_OF_32(Renderable::matrix), &renderable->matrix, 0);
        commandList->SetGraphicsRoot32BitConstants(1, SIZE_OF_32(XMFLOAT3), &colors[(data->startIndex + k) % 8], 0);
        commandList->SetGraphicsRootDescriptorTable(2, renderContext->GetExternalSRVGPUHandle(renderable->textureID));
        commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
    }
}
