#pragma once
#include "pch.h"
using namespace DirectX;
namespace R
{
	namespace ECS
	{
		struct Pos : public XMFLOAT3
		{
			inline Pos(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) : XMFLOAT3(_x, _y, _z) {}
			static const std::uint64_t uid = 1;
		};

		struct Rotation : public XMFLOAT4
		{
			inline Rotation(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f, float _w = 1.0f) : XMFLOAT4(_x, _y, _z, _w) {}
			static const std::uint64_t uid = 1 << 1;
		};

		struct Scale : public XMFLOAT3
		{
			inline Scale(float _x = 1.0f, float _y = 1.0f, float _z = 1.0f) : XMFLOAT3(_x, _y, _z) {}
			static const std::uint64_t uid = 1 << 2;
		};
	}
}