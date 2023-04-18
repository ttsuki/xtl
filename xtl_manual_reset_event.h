/// @file
/// @brief  xtl::manual_reset_event
/// @author ttsuki
#pragma once

#include <chrono>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace xtl
{
    class manual_reset_event final
    {
        std::mutex mutex_{};
        std::condition_variable cv_{};
        std::atomic_bool signal_{};

    public:
        manual_reset_event() = default;
        manual_reset_event(const manual_reset_event& other) = delete;
        manual_reset_event(manual_reset_event&& other) noexcept = delete;
        manual_reset_event& operator=(const manual_reset_event& other) = delete;
        manual_reset_event& operator=(manual_reset_event&& other) noexcept = delete;
        ~manual_reset_event() = default;

        void notify_signal()
        {
            std::unique_lock lock(mutex_);
            signal_.store(true);
            cv_.notify_all();
        }

        void reset_signal()
        {
            std::unique_lock lock(mutex_);
            signal_.store(false);
        }

        void wait()
        {
            std::unique_lock lock(mutex_);
            cv_.wait(lock, [this] { return signal_.load(); });
        }

        template <class rep, class ratio>
        bool wait_for(const std::chrono::duration<rep, ratio>& rel_time)
        {
            std::unique_lock lock(mutex_);
            return cv_.wait_for(lock, rel_time, [this] { return signal_.load(); });
        }

        template <class clock, class duration>
        bool wait_until(const std::chrono::time_point<clock, duration>& timeout_time)
        {
            std::unique_lock lock(mutex_);
            return cv_.wait_until(lock, timeout_time, [this] { return signal_.load(); });
        }
    };
}
