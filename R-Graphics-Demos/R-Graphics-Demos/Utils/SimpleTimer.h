#pragma once
#include "pch.h"
namespace R
{
	namespace Utils
	{
		class SimpleTimer
		{
		public:
			inline SimpleTimer(const std::string& _name) :m_name(_name + ": Elapsed {} ms"), m_start(std::chrono::system_clock::now()) {};
			inline ~SimpleTimer() { R_LOG_DEBUG(m_name, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_start).count()); }
		private:
			std::string											m_name;
			std::chrono::time_point<std::chrono::system_clock>	m_start;
		};
	}
}

#ifdef _DEBUG
#define R_TIMER(x) R::Utils::SimpleTimer _##x(#x)
#else
#define R_TIMER(x)
#endif // _DEBUG
