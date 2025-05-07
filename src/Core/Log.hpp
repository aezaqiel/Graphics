#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace Graphics {

    class Log
    {
    public:
        static void Init();
        static void Shutdown();

        inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

    private:
        static std::shared_ptr<spdlog::logger> s_Logger;
    };

}

#ifndef NDEBUG
    #define LOG_TRACE(...)      ::Graphics::Log::GetLogger()->trace(__VA_ARGS__)
    #define LOG_DEBUG(...)      ::Graphics::Log::GetLogger()->debug(__VA_ARGS__)
    #define LOG_INFO(...)       ::Graphics::Log::GetLogger()->info(__VA_ARGS__)
    #define LOG_WARN(...)       ::Graphics::Log::GetLogger()->warn(__VA_ARGS__)
    #define LOG_ERROR(...)      ::Graphics::Log::GetLogger()->error(__VA_ARGS__)
    #define LOG_CRITICAL(...)   ::Graphics::Log::GetLogger()->critical(__VA_ARGS__)
#else
    #define LOG_TRACE(...)
    #define LOG_DEBUG(...)
    #define LOG_INFO(...)
    #define LOG_WARN(...)
    #define LOG_ERROR(...)
    #define LOG_CRITICAL(...)
#endif
