#pragma once
#include "pch.h"
#include "DX12Helper.h"
#include "ECS/World.h"
#include "ECS/PredefinedComponents.h"
#include "Renderables.h"

using Microsoft::WRL::ComPtr;
namespace R
{
	namespace Rendering
	{
		class FrameResource
		{
		public:
			enum class FrameResourceState : uint32_t
			{
				e_inUse,
				e_free,
				e_FrameResourceStateCount
			};
			FrameResource();
			~FrameResource();
			void Submit(ID3D12CommandQueue* cmdQ, ID3D12Fence* fence, uint64_t& frameNumber);
			void Release(ID3D12Fence* fence);

			inline Renderable * const GetRenderable(uint32_t index) { assert(index < ECS::MAX_ENTITIES_PER_ARCHETYPE); return &m_renderables[index]; }
			inline FrameResourceState GetState() const { return m_state; }
			inline void SetCount(const uint32_t& count) { m_count = count; }
			inline uint32_t GetCount() { return m_count; }
		private:
			Renderable*									m_renderables = new Renderable[ECS::MAX_ENTITIES_PER_ARCHETYPE]; 
			uint32_t									m_count;
			FrameResourceState							m_state;
			// Synchronization objects.
			uint64_t									m_fenceValue;
			HANDLE										m_eventHandle;
		};
	}
}