
#pragma once

#include "Guid.h"
#include "spdlog/spdlog.h"

namespace strafe
{
	class Logger
	{
	public:
		/**
		* \brief Initializes spdlog
		*/
		static void Init();
		/**
		 * \brief Shutdown spdlog
		 */
		static void Shutdown();

		/**
		 * \brief Getter for Meow Core logger
		 * \return Returns a spdlog logger
		 */
		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }

		/**
		 * \brief Getter for Meow Client logger
		 * \return Returns a spdlog logger
		 */
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		/**
		 * \brief The core logger used for in-engine logging
		 */
		static inline std::shared_ptr<spdlog::logger> s_CoreLogger;
		/**
		 * \brief The client used for outside logging, such as C#
		 */
		static inline std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}
#define RD_CORE_TRACE(...) ::strafe::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define RD_CORE_INFO(...) ::strafe::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define RD_CORE_WARN(...) ::strafe::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define RD_CORE_ERROR(...) ::strafe::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define RD_CORE_FATAL(...) ::strafe::Logger::GetCoreLogger()->critical(__VA_ARGS__)

#define RD_CLIENT_TRACE(...) ::strafe::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define RD_CLIENT_INFO(...) ::strafe::Logger::GetClientLogger()->info(__VA_ARGS__)
#define RD_CLIENT_WARN(...) ::strafe::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define RD_CLIENT_ERROR(...) ::strafe::Logger::GetClientLogger()->error(__VA_ARGS__)
#define RD_CLIENT_FATAL(...) ::strafe::Logger::GetClientLogger()->critical(__VA_ARGS__)

#include "spdlog/fmt/bundled/ostream.h"
#include "Strafe/Math/StrafeMath.h"
#define RD_LOG_OVERLOAD_USERTYPE(type, var, format)\
inline std::ostream& operator<<(std::ostream& os, const type& var){\
	return os << format;\
}\
template <> struct fmt::formatter<type> : ostream_formatter {}

RD_LOG_OVERLOAD_USERTYPE(glm::ivec2, vec, "(" << vec.x << "," << vec.y << ")");
RD_LOG_OVERLOAD_USERTYPE(glm::ivec3, vec, "(" << vec.x << "," << vec.y << ", " << vec.z <<  ")");
RD_LOG_OVERLOAD_USERTYPE(glm::ivec4, vec, "(" << vec.x << "," << vec.y << ", " << vec.z << ", " << vec.w << ")");
RD_LOG_OVERLOAD_USERTYPE(glm::dvec2, vec, "(" << vec.x << "," << vec.y << ")");
RD_LOG_OVERLOAD_USERTYPE(glm::dvec3, vec, "(" << vec.x << "," << vec.y << ", " << vec.z << ")");
RD_LOG_OVERLOAD_USERTYPE(glm::dvec4, vec, "(" << vec.x << "," << vec.y << ", " << vec.z << ", " << vec.w << ")");
RD_LOG_OVERLOAD_USERTYPE(glm::vec2, vec, "(" << vec.x << "," << vec.y << ")");
RD_LOG_OVERLOAD_USERTYPE(glm::vec3, vec, "(" << vec.x << "," << vec.y << ", " << vec.z << ")");
RD_LOG_OVERLOAD_USERTYPE(glm::vec4, vec, "(" << vec.x << "," << vec.y << ", " << vec.z << ", " << vec.w << ")");

RD_LOG_OVERLOAD_USERTYPE(strafe::Guid, id, id.m_RawId);