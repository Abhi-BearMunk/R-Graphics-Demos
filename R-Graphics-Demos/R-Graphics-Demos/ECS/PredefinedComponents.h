#pragma once
#include "pch.h"
using namespace DirectX;
// UID -> 0 to 15
namespace R
{
	using namespace Constants;
	namespace ECS
	{
		struct Pos : public XMFLOAT3
		{
			inline Pos(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) : XMFLOAT3(_x, _y, _z) {}
			static const std::uint64_t uid = 0;
		};

		struct Rotation : public XMFLOAT4
		{
			inline Rotation() : Rotation(0, 0, 0, 1) {}
			inline Rotation(float _x, float _y, float _z) { XMStoreFloat4(this, XMQuaternionRotationRollPitchYaw(_z * DegToRad, _x * DegToRad, _y * DegToRad)); }
			inline Rotation(float _x, float _y, float _z, float _w) : XMFLOAT4(_x, _y, _z, _w) {}
			static const std::uint64_t uid = 1;
		};

		struct Scale : public XMFLOAT3
		{
			inline Scale(float _x = 1.0f, float _y = 1.0f, float _z = 1.0f) : XMFLOAT3(_x, _y, _z) {}
			static const std::uint64_t uid = 2;
		};
	}
}