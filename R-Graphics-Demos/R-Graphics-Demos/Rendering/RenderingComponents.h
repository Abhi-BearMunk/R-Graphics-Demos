#pragma once
#include "pch.h"
// UID -> 1 << 16 to 1 << 31
namespace R
{
	namespace Rendering
	{
		struct Mesh
		{
			uint32_t meshId;
			uint32_t albedoTextureId;
			uint32_t normalTextureId;
			uint32_t specularTextureId;
			static const std::uint64_t uid = 1 << 16;
		};
	}
}