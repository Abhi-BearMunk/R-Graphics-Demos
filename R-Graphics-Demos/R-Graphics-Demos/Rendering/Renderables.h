#pragma once
#include "pch.h"
namespace R
{
	namespace Rendering
	{
		struct Renderable
		{
			XMFLOAT4X4 matrix;
			std::uint32_t textureID;
		};
	}
}