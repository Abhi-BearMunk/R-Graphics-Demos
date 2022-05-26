#pragma once
namespace R
{
	namespace Test
	{
		struct Velocity
		{
			float x;
			float y;
			float z;
			static const uint64_t uid = 1Ui64 << 63;
		};
	}
}