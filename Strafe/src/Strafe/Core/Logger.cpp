

#include "strafepch.h"

#include "Logger.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace strafe
{
	void Logger::Init()
	{
		spdlog::set_pattern("[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("Strafe");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("Strafe Runtime");
		s_ClientLogger->set_level(spdlog::level::trace);
	}

	void Logger::Shutdown()
	{
		spdlog::shutdown();
	}
}