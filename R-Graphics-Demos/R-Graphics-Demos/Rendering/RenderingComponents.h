#pragma once
#include "pch.h"
// UID -> 16 to 31
namespace R
{
	namespace Rendering
	{
		struct Mesh
		{
			uint32_t meshId;
			uint32_t textureIdStart;
			static const std::uint64_t uid = 16;
		};
	}
}