#include "pch.h"
#include "Logger.h"

std::shared_ptr<spdlog::logger> R::Utils::Logger::file_logger;

R::Utils::Logger::Logger()
{
	// TODO : Kinda hacky, but for now ensures only one instance of logger
	assert(file_logger.use_count() == 0);
	auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
	std::string fName = "Logs/" + std::format("{:%Y-%m-%d-%X}", time) + ".txt";
	std::replace(fName.begin(), fName.end(), ':', '-');
	file_logger = spdlog::basic_logger_mt("file_logger", fName);
#ifdef _DEBUG
	file_logger->set_level(spdlog::level::debug);
#else
	file_logger->set_level(spdlog::level::info);
#endif // _DEBUG
	spdlog::set_default_logger(file_logger);
	file_logger->info("==========Start Log==========");
}

R::Utils::Logger::~Logger()
{
	file_logger->flush();
}
