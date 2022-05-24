#include "pch.h"
#include "GraphicsUtils.h"

void R::Rendering::CommandCreateBufferFromData(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12Resource** dst, ID3D12Resource** uploadBuffer, void* data, size_t sizeInBytes, D3D12_RESOURCE_STATES stateAfter)
{
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes);
    CD3DX12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_HEAP_PROPERTIES uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    LogErrorIfFailed(device->CreateCommittedResource(
        &defaultHeap,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(dst)));

    LogErrorIfFailed(device->CreateCommittedResource(
        &uploadHeap,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer)));

    // Describe the data we want to copy into the default buffer.
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = data;
    subResourceData.RowPitch = sizeInBytes;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
    // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
    // the intermediate upload heap data will be copied to mBuffer.
    UpdateSubresources<1>(cmdList, *dst, *uploadBuffer, 0, 0, 1, &subResourceData);
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(*dst,
        D3D12_RESOURCE_STATE_COPY_DEST, stateAfter);
    cmdList->ResourceBarrier(1, &barrier);
}
