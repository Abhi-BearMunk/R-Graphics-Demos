#include "pch.h"
#include "RenderSystem.h"
#include "FrameResource.h"

R::Rendering::RenderSystem::RenderSystem(const std::uint32_t width, const std::uint32_t height, const HWND windowHandle, Job::JobSystem& jobSystem, const ResourceData* externalResources)
	:
	m_renderContext(width, height, windowHandle, jobSystem.GetNumWorkers()),
	m_pJobSystem(&jobSystem),
	m_basePass(&m_renderContext, m_pJobSystem),
	m_beginFrame(m_renderContext.GetDevice(), L"BeginFrame"),
	m_endFrame(m_renderContext.GetDevice(), L"EndFrame")
{
	LogErrorIfFailed(m_beginFrame.GetCommandList()->Close());
	LogErrorIfFailed(m_endFrame.GetCommandList()->Close());

	ThreadRenderContext<1> temp(m_renderContext.GetDevice(), L"InitializeResources");
	// TODO: Init Passes
	m_basePass.Init(temp.GetCommandList());

	LogErrorIfFailed(temp.GetCommandList()->Close());
	ID3D12CommandList* ppCommandLists[] = { temp.GetCommandList() };
	m_renderContext.GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create and start resource upload
	if (externalResources)
	{
		for (std::uint32_t i = 0; i < m_pJobSystem->GetNumWorkers(); i++)
		{
			m_renderContext.GetThreadContext(i)->Reset(0);
		}
		CommandUploadResourcesToGPU(externalResources);
	}
	// Wait for GPU to finish work
	m_renderContext.WaitForGPU();
}

R::Rendering::RenderSystem::~RenderSystem()
{
}


void R::Rendering::RenderSystem::Render()
{
	if (m_renderContext.GetCurrentFrameResource()->GetCount() == 0)
		return;
	m_renderContext.GetCurrentFrameResource()->Release(m_renderContext.GetFence());
	assert(m_renderContext.GetCurrentFrameResource()->GetState() == FrameResource::FrameResourceState::e_free);

	// ========== Begin Frame ==========
	{
		m_beginFrame.Reset(m_renderContext.GetFrameIndex());
		auto commandList = m_beginFrame.GetCommandList();

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderContext.GetCurrentRenderTarget(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier);

		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		commandList->ClearRenderTargetView(m_renderContext.GetCurrentRTVHandle(), clearColor, 0, nullptr);
		commandList->ClearDepthStencilView(m_renderContext.GetCurrentDSVHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		LogErrorIfFailed(commandList->Close());
		ID3D12CommandList* ppCommandLists[] = { commandList };
		m_renderContext.GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}


	// ========== Passes ==========
	// Todo Move this to affinity based job if needed?
	for (std::uint32_t i = 0; i < m_pJobSystem->GetNumWorkers(); i++)
	{
		m_renderContext.GetThreadContext(i)->Reset(m_renderContext.GetFrameIndex());
		m_renderContext.GetThreadContext(i)->GetCommandList()->RSSetViewports(1, m_renderContext.GetViewPort());
		m_renderContext.GetThreadContext(i)->GetCommandList()->RSSetScissorRects(1, m_renderContext.GetScissorRect());
	}

	// TODO : Submit GPU Work
	// ...
	m_basePass.Update();
	m_basePass.WaitForCompletion();

	// ========== End Frame ==========
	CloseAndExecuteThreadCommandLists();

	{
		m_endFrame.Reset(m_renderContext.GetFrameIndex());
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderContext.GetCurrentRenderTarget(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_endFrame.GetCommandList()->ResourceBarrier(1, &barrier);
		LogErrorIfFailed(m_endFrame.GetCommandList()->Close());
		ID3D12CommandList* ppCommandLists[] = { m_endFrame.GetCommandList() };
		m_renderContext.GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}

	LogErrorIfFailed(m_renderContext.GetSwapChain()->Present(1, 0));

	m_renderContext.GetCurrentFrameResource()->Submit(m_renderContext.GetCommandQueue(), m_renderContext.GetFence(), m_renderContext.GetFrameNumber());
}

void R::Rendering::RenderSystem::CommandUploadResourcesToGPU(const ResourceData* resourceData)
{
	std::uint32_t numTextures = resourceData->textureDatas.size();

	// TODO: This should be elsewhere
	m_renderContext.CreateCbvSrvUavHeap(numTextures);
	ComPtr<ID3D12Resource>* textureUploaders = new ComPtr<ID3D12Resource>[numTextures];

	std::uint32_t updateBatchSize = std::max(10u, numTextures / 64);
	std::uint32_t numTextureJobs = static_cast<std::uint32_t>(ceil(numTextures / static_cast<float>(updateBatchSize)));
	// Setup jobn params
	ResourceUploadJobConstData constData{ &m_renderContext, resourceData, textureUploaders };
	ResourceUploadJobData* textureJobDatas = new ResourceUploadJobData[numTextureJobs];
	Job::JobSystem::JobDesc* textureJobDescs = new Job::JobSystem::JobDesc[numTextureJobs];

	Job::JobSystem::JobCounter jobCounter;
	jobCounter.counter = numTextureJobs;

	for (int i = 0; i < numTextureJobs; i++)
	{
		textureJobDescs[i].jobFunc = &TextureUploadJobFunc;
		textureJobDescs[i].param = &textureJobDatas[i];
		textureJobDescs[i].pCounter = &jobCounter;

		textureJobDatas[i].constData = &constData;
		textureJobDatas[i].startIndex = i * updateBatchSize;
		textureJobDatas[i].batchSize = std::min(updateBatchSize, numTextures - textureJobDatas[i].startIndex);
	}

	m_pJobSystem->KickJobsWithPriorityAndWait(textureJobDescs, numTextureJobs);
	CloseAndExecuteThreadCommandLists();
	delete[] textureJobDatas;
	delete[] textureJobDescs;
}

void R::Rendering::RenderSystem::TextureUploadJobFunc(void* param, std::uint32_t tid)
{
	ResourceUploadJobData* data = reinterpret_cast<ResourceUploadJobData*>(param);

	auto renderContext			= data->constData->renderContext;
	auto device					= renderContext->GetDevice();
	auto commandList			= renderContext->GetThreadContext(tid)->GetCommandList();

	auto resourceData			= data->constData->resourceData;
	auto textureUploader		= data->constData->textureUploaders;

	auto srvHandle				= renderContext->GetExternalSRVCPUHandle(data->startIndex);

	for (std::uint32_t textureIndex = data->startIndex; textureIndex < data->startIndex + data->batchSize; textureIndex++)
	{
		// Describe and create a Texture2D.
		const TextureData* texData = resourceData->GetTextureData(textureIndex);
		const ScratchImage& scratchImage = texData->scratchImage;
		const TexMetadata& metaData = scratchImage.GetMetadata();

		CD3DX12_RESOURCE_DESC texDesc(
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,
			metaData.width,
			metaData.height,
			1,
			static_cast<UINT16>(metaData.mipLevels),
			metaData.format,
			1,
			0,
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_NONE);

		ID3D12Resource** textureResource = renderContext->GetExternalResourceAddr(textureIndex);
		CD3DX12_HEAP_PROPERTIES defaultHeapDesc{ D3D12_HEAP_TYPE_DEFAULT };
		LogErrorIfFailed(device->CreateCommittedResource(
			&defaultHeapDesc,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(textureResource)));

		NAME_D3D12_OBJECT_INDEXED(*textureResource, textureIndex);

		std::vector<D3D12_SUBRESOURCE_DATA> subresources(scratchImage.GetImageCount());

		const Image* pImages = scratchImage.GetImages();

		for (int i = 0; i < scratchImage.GetImageCount(); ++i) {

			auto& subresource = subresources[i];
			subresource.RowPitch = pImages[i].rowPitch;
			subresource.SlicePitch = pImages[i].slicePitch;
			subresource.pData = pImages[i].pixels;
		}

		{
			ID3D12Resource** intermediateResource = textureUploader[textureIndex].GetAddressOf();
			UINT64 requiredSize = GetRequiredIntermediateSize(*textureResource, 0, subresources.size());

			CD3DX12_HEAP_PROPERTIES uploadHeapDesc{ D3D12_HEAP_TYPE_UPLOAD };
			auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(requiredSize);

			device->CreateCommittedResource(
				&uploadHeapDesc,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(intermediateResource)
			);

			// Copy data to the intermediate upload heap and then schedule a copy
			// from the upload heap to the Texture2D.
			UpdateSubresources(commandList, *textureResource, *intermediateResource, 0, 0, subresources.size(), subresources.data());
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(*textureResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			commandList->ResourceBarrier(1, &barrier);
		}

		// Describe and create an SRV.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = metaData.format;
		srvDesc.Texture2D.MipLevels = metaData.mipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		device->CreateShaderResourceView(*textureResource, &srvDesc, srvHandle);

		// Move to the next descriptor slot.
		renderContext->OffsetCBVSRVUAVCPUHandle(srvHandle);

		// Update the TexID
		texData->texID->m_id = textureIndex;
	}
}

void R::Rendering::RenderSystem::CloseAndExecuteThreadCommandLists()
{
	for (std::uint32_t i = 0; i < m_pJobSystem->GetNumWorkers(); i++)
	{
		LogErrorIfFailed(m_renderContext.GetThreadContext(i)->GetCommandList()->Close());
	}
	m_renderContext.GetCommandQueue()->ExecuteCommandLists(static_cast<UINT>(m_pJobSystem->GetNumWorkers()), m_renderContext.GetCommandLists());
}


