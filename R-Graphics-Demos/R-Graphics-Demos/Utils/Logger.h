#pragma once
#include "pch.h"
namespace R
{
	namespace Utils
	{
		class Logger
		{
		public:
			Logger();
			~Logger();
			static std::shared_ptr<spdlog::logger> file_logger;
		};
	}
#define R_LOG_INFO(...) R::Utils::Logger::file_logger->info(__VA_ARGS__)
#define R_LOG_ERROR(...) R::Utils::Logger::file_logger->error(__VA_ARGS__)
#ifdef _DEBUG
#define R_LOG_WARN(...) R::Utils::Logger::file_logger->warn(__VA_ARGS__)
#define R_LOG_DEBUG(...) R::Utils::Logger::file_logger->debug(__VA_ARGS__)
#else
#define R_LOG_WARN(...)
#define R_LOG_DEBUG(...)
#endif // _DEBUG


}
