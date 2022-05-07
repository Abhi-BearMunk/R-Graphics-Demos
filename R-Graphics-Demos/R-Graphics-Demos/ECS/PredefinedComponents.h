#pragma once
#include <cstdint>
namespace R
{
	namespace ECS
	{
		struct Pos
		{
			float x;
			float y;
			float z;
			static const uint64_t uid = 1;
		};

		struct Rotation
		{
			float x;
			float y;
			float z;
			static const uint64_t uid = 1 << 1;
		};

		struct Scale
		{
			float x;
			float y;
			float z;
			static const uint64_t uid = 1 << 2;
		};
	}
}