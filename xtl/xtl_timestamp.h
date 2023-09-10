/// @file
/// @brief  xtl::timestamp - Simple timestamp type
/// @author (C) 2021 ttsuki
/// Distributed under the Boost Software License, Version 1.0.

#pragma once

#include <chrono>
#include <string>
#include <ctime>

namespace xtl
{
    struct timestamp
    {
        using unit = std::chrono::microseconds;
        using value_type = int64_t;

        /// Ticks per second
        static constexpr inline value_type ticks_per_second = std::chrono::duration_cast<unit>(std::chrono::seconds(1)).count();

        /// Elapsed tick count from unix-time epoch
        value_type tick;

        [[nodiscard]] static inline timestamp now() noexcept
        {
            using namespace std::chrono;
            static value_type epochOffset = []
            {
                auto st = system_clock::now() - system_clock::from_time_t(0);
                auto ht = high_resolution_clock::now().time_since_epoch();
                return static_cast<value_type>((duration_cast<unit>(st) - duration_cast<unit>(ht)).count());
            }();

            return timestamp{duration_cast<unit>(high_resolution_clock::now().time_since_epoch()).count() + epochOffset};
        }

        [[nodiscard]] long double to_seconds() const noexcept
        {
            return static_cast<double>(static_cast<long double>(tick) / static_cast<long double>(ticks_per_second));
        }

        [[nodiscard]] std::chrono::system_clock::time_point to_system_clock_time_point() const noexcept
        {
            using namespace std::chrono;
            return system_clock::from_time_t(0) + duration_cast<system_clock::duration>(unit(tick));
        }

        [[nodiscard]] std::string to_localtime_string() const noexcept
        {
            auto time = std::chrono::system_clock::to_time_t(to_system_clock_time_point());
            std::tm tm{};
#if defined(_MSC_VER)
            localtime_s(&tm, &time);
#else
            localtime_r(&time, &tm); // since C23
            //localtime_s(&time, &tm); // since C11
            //tm = *localtime(&time);
#endif

            char str[64]{};
            size_t len = std::strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", &tm);

            static_assert(unit::period::num == 1 && unit::period::den == 1000000);
            std::snprintf(str + len, sizeof(str) - len, ".%06d", static_cast<int>(tick * unit::period::num % unit::period::den));
            return std::string(str);
        }

        bool operator ==(timestamp rhs) const { return tick == rhs.tick; }
        bool operator !=(timestamp rhs) const { return tick != rhs.tick; }
        bool operator <(timestamp rhs) const { return tick < rhs.tick; }
        bool operator >(timestamp rhs) const { return tick > rhs.tick; }
        bool operator <=(timestamp rhs) const { return tick <= rhs.tick; }
        bool operator >=(timestamp rhs) const { return tick >= rhs.tick; }
    };

    static_assert(std::is_standard_layout_v<timestamp>);
    static_assert(std::is_trivial_v<timestamp>);
    static_assert(sizeof(timestamp) == sizeof(int64_t));
}
