/// @file
/// @brief  xtl spin_lock_mutex
/// @author (C) 2021-2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <atomic>
#include <mutex>
#include <thread>
#include <exception>

namespace
XTL_NAMESPACE
{
    /// Simple spin-lock mutex.
    class spin_lock_mutex final
    {
        std::atomic_flag state_{ATOMIC_FLAG_INIT};

    public:
        spin_lock_mutex() = default;
        spin_lock_mutex(const spin_lock_mutex& other) = delete;
        spin_lock_mutex(spin_lock_mutex&& other) noexcept = delete;
        spin_lock_mutex& operator=(const spin_lock_mutex& other) = delete;
        spin_lock_mutex& operator=(spin_lock_mutex&& other) noexcept = delete;
        ~spin_lock_mutex() = default;

        inline bool try_lock()
        {
            return !state_.test_and_set(std::memory_order_acquire);
        }

        inline void lock()
        {
            if (!try_lock())
            {
                for (size_t i = 0; !try_lock(); i++)
                    if ((i & 0xFFFF) == 0)
                        std::this_thread::yield();
            }
        }

        inline void unlock()
        {
#ifndef NDEBUG
            if (!state_.test_and_set(std::memory_order_relaxed)) std::terminate();
#endif
            state_.clear(std::memory_order_release);
        }
    };

    /// Recursive spin-lock mutex.
    class recursive_spin_lock_mutex final
    {
        using thread_id = std::thread::id;
        std::atomic_flag state_{ATOMIC_FLAG_INIT};
        std::atomic<thread_id> owner_{};
        size_t lock_count_{};

    public:
        recursive_spin_lock_mutex() = default;
        recursive_spin_lock_mutex(const recursive_spin_lock_mutex& other) = delete;
        recursive_spin_lock_mutex(recursive_spin_lock_mutex&& other) noexcept = delete;
        recursive_spin_lock_mutex& operator=(const recursive_spin_lock_mutex& other) = delete;
        recursive_spin_lock_mutex& operator=(recursive_spin_lock_mutex&& other) noexcept = delete;
        ~recursive_spin_lock_mutex() = default;

        inline bool try_lock()
        {
            thread_id self = std::this_thread::get_id();

            if (!state_.test_and_set(std::memory_order_acquire))
            {
                owner_.store(self, std::memory_order_relaxed);
                ++lock_count_;
                return true;
            }

            if (owner_.load(std::memory_order_relaxed) == self)
            {
                ++lock_count_;
                return true;
            }

            return false;
        }

        inline void lock()
        {
            for (size_t i = 0; !try_lock(); i++)
                if ((i & 0xFFFF) == 0)
                    std::this_thread::yield();
        }

        inline void unlock()
        {
            thread_id self = std::this_thread::get_id();

#ifndef NDEBUG
            if (owner_.load(std::memory_order_relaxed) != self)
                std::terminate();
#endif

            if (--lock_count_ == 0)
            {
                owner_.store({}, std::memory_order_relaxed);
                state_.clear(std::memory_order_release);
            }
        }
    };

    using std::lock_guard;
}
