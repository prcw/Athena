#pragma once

#include <chrono>
#include <iomanip>

#include "../../resources/color.hpp"

class Debug
{
public:
    static __forceinline void AllocateConsole()
    {
        AllocConsole();

        FILE* file;
        freopen_s(&file, ("CONOUT$"), ("w"), stdout);
    }

    template <typename T>
    static __forceinline void AppendToStream(std::ostringstream& stream, T&& value)
    {
        stream << std::forward<T>(value);
    }

    template <typename T, typename... Args>
    static __forceinline void AppendToStream(std::ostringstream& stream, T&& first, Args&&... rest)
    {
        stream << std::forward<T>(first);
        AppendToStream(stream, std::forward<Args>(rest)...);
    }

    template <typename... Args>
    static __forceinline void Log(Args&&... args)
    {
//#ifdef _DEBUG
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        struct tm local_time;
        localtime_s(&local_time, &time);

        std::ostringstream timestamp_stream;
        timestamp_stream << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S")
            << '.' << std::setw(3) << std::setfill('0') << milliseconds.count();

        std::ostringstream message_stream;
        AppendToStream(message_stream, std::forward<Args>(args)...);

        std::cout << dye::aqua("[" + timestamp_stream.str() + "] ") << message_stream.str() << std::endl;
//#endif
    }
};