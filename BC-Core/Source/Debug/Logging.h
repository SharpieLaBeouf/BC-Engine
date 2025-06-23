#pragma once

// Core Headers

// C++ Standard Library Headers
#include <mutex>
#include <string>
#include <vector>
#include <memory>

// External Vendor Library Headers
#pragma warning( push )
#pragma warning( disable : 4996 )
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/log_msg.h>

namespace BC {

    // Structure to store a log entry's details.
    struct LogEntry 
    {
        std::string text;
        spdlog::level::level_enum level;
        std::string logger_name;
    };

    class CustomLoggingSink : public spdlog::sinks::base_sink<std::mutex> 
    {

    public:

        // Singleton accessor
        static CustomLoggingSink& GetInstance() {
            static CustomLoggingSink instance;
            return instance;
        }

        // Returns all log entries
        const std::vector<LogEntry>& GetLogs() const { return m_LogBuffer; }
        void Clear() { m_LogBuffer.clear(); }

    protected:

        // Called when a new log message is emitted
        void sink_it_(const spdlog::details::log_msg& msg) override {
            spdlog::memory_buf_t formatted;
            base_sink<std::mutex>::formatter_->format(msg, formatted);
            LogEntry entry;
            entry.text = fmt::to_string(formatted);
            entry.level = msg.level;
            entry.logger_name = msg.logger_name.data(); // logger name is stored in msg.logger_name
            m_LogBuffer.push_back(std::move(entry));
        }

        void flush_() override {}

    private:

        std::vector<LogEntry> m_LogBuffer;
    };

	class LoggingSystem {

	public:

		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetApplicationLogger() { return s_ApplicationLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetScriptingLogger() { return s_ScriptingLogger; }

	private:
    
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ApplicationLogger;
		static std::shared_ptr<spdlog::logger> s_ScriptingLogger;

	};

}

#define BC_CORE_INFO(...)   ::BC::LoggingSystem::GetCoreLogger()->info(__VA_ARGS__)
#define BC_CORE_TRACE(...)  ::BC::LoggingSystem::GetCoreLogger()->trace(__VA_ARGS__)
#define BC_CORE_WARN(...)   ::BC::LoggingSystem::GetCoreLogger()->warn(__VA_ARGS__)
#define BC_CORE_ERROR(...)  ::BC::LoggingSystem::GetCoreLogger()->error(__VA_ARGS__)
#define BC_CORE_FATAL(...)  ::BC::LoggingSystem::GetCoreLogger()->critical(__VA_ARGS__)

#define BC_APP_INFO(...)   ::BC::LoggingSystem::GetApplicationLogger()->info(__VA_ARGS__)
#define BC_APP_TRACE(...)  ::BC::LoggingSystem::GetApplicationLogger()->trace(__VA_ARGS__)
#define BC_APP_WARN(...)   ::BC::LoggingSystem::GetApplicationLogger()->warn(__VA_ARGS__)
#define BC_APP_ERROR(...)  ::BC::LoggingSystem::GetApplicationLogger()->error(__VA_ARGS__)
#define BC_APP_FATAL(...)  ::BC::LoggingSystem::GetApplicationLogger()->critical(__VA_ARGS__)

#define BC_SCRIPT_INFO(...)   ::BC::LoggingSystem::GetScriptingLogger()->info(__VA_ARGS__)
#define BC_SCRIPT_TRACE(...)  ::BC::LoggingSystem::GetScriptingLogger()->trace(__VA_ARGS__)
#define BC_SCRIPT_WARN(...)   ::BC::LoggingSystem::GetScriptingLogger()->warn(__VA_ARGS__)
#define BC_SCRIPT_ERROR(...)  ::BC::LoggingSystem::GetScriptingLogger()->error(__VA_ARGS__)
#define BC_SCRIPT_FATAL(...)  ::BC::LoggingSystem::GetScriptingLogger()->critical(__VA_ARGS__)

#pragma warning( pop )