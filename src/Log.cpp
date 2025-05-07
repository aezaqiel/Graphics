#include "Log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Graphics {

    std::shared_ptr<spdlog::logger> Log::s_Logger;

    void Log::Init()
    {
        spdlog::set_pattern("[%H:%M:%S %z] [%^%l%$] [thread %t] %v");
        s_Logger = spdlog::stdout_color_mt("LOG");
        s_Logger->set_level(spdlog::level::trace);
    }

    void Log::Shutdown()
    {
        s_Logger.reset();
        spdlog::drop_all();
    }

}
