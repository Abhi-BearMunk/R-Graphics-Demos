#pragma once
#include "pch.h"
using namespace DirectX;
namespace R
{
	namespace Rendering
	{
		struct TextureID
		{
		public:
			inline uint32_t Get() { assert((m_id < UINT32_MAX) && "Texture not commited to memory"); return m_id; }
		private:
			std::uint32_t		m_id = UINT32_MAX;
			friend class RenderSystem;
		};

		struct TextureData
		{
			TextureID*				texID;
			ScratchImage			scratchImage;
		};

		struct ResourceData
		{
			inline const TextureData* GetTextureData(std::uint32_t index) const { return &textureDatas[index]; }
			std::vector<TextureData> textureDatas;
		};
	}
}