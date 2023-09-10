/// @file
/// @brief  xtl concurrent_queue - a queue for producer/consumer pattern.
/// @author (C) 2021 ttsuki
/// Distributed under the Boost Software License, Version 1.0.

#pragma once
#include <cstddef>
#include <queue>
#include <optional>
#include <limits>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

namespace xtl
{
    enum struct concurrent_queue_if_limit_reached
    {
        block,
        drop_last,
        drop_first,
    };

    template <class T>
    class concurrent_queue final
    {
    public:
        using if_limit_reached = concurrent_queue_if_limit_reached;

    private:
        mutable std::recursive_mutex mutex_{};
        const size_t limit_;
        const if_limit_reached dropPolicy_;
        std::condition_variable_any can_produce_;
        std::condition_variable_any can_consume_;
        std::queue<std::optional<T>> queue_{};

    public:
        concurrent_queue(
            size_t limit = std::numeric_limits<size_t>::max(),
            if_limit_reached mode = if_limit_reached::block)
            : limit_(limit)
            , dropPolicy_(mode)
        {
            switch (mode)
            {
            case if_limit_reached::block:
            case if_limit_reached::drop_last:
            case if_limit_reached::drop_first:
                break;
            default:
                throw std::invalid_argument("mode");
            }
        }

        [[nodiscard]] bool closed() const noexcept
        {
            std::unique_lock lock(mutex_);
            return !queue_.empty() && !queue_.front();
        }

        [[nodiscard]] bool empty() const noexcept
        {
            std::unique_lock lock(mutex_);
            return closed() || queue_.empty();
        }

        [[nodiscard]] size_t capacity() const noexcept
        {
            return limit_;
        }

        [[nodiscard]] size_t size() const noexcept
        {
            std::unique_lock lock(mutex_);
            return closed() ? 0 : queue_.size();
        }

        /// Pushes value.
        template <class... U>
        bool push(U&& ...val)
        {
            std::unique_lock lock(mutex_);
            if (closed()) { return false; }
            if (dropPolicy_ == if_limit_reached::block) { can_produce_.wait(lock, [&] { return queue_.size() < limit_; }); }
            if (dropPolicy_ == if_limit_reached::drop_last) { if (queue_.size() == limit_) { return false; } }
            if (dropPolicy_ == if_limit_reached::drop_first) { if (queue_.size() == limit_) { queue_.pop(); } }
            queue_.emplace(std::in_place, std::forward<U>(val)...);
            can_consume_.notify_all();
            return true;
        }

        /// Closes queue.
        void close()
        {
            std::unique_lock lock(mutex_);
            queue_.emplace(std::nullopt);
            can_consume_.notify_all();
        }

        /// Tries pop value, may returns nullopt if queue is empty or closed.
        [[nodiscard]] std::optional<T> try_pop()
        {
            std::unique_lock lock(mutex_);
            if (closed() || queue_.empty()) return std::nullopt;

            std::optional<T> ret = std::move(queue_.front());
            queue_.pop();
            can_produce_.notify_all();
            return ret;
        }


        /// Waits for value.
        /// may return nullopt if queue is closed.
        [[nodiscard]] std::optional<T> pop_wait()
        {
            std::unique_lock lock(mutex_);
            std::optional<T> ret = std::nullopt;
            can_consume_.wait(lock, [&] { return closed() || (ret = try_pop()).has_value(); });
            return ret;
        }

        /// Waits for value.
        /// may returns nullopt if queue is closed, or empty till timed out.
        template <class Rep, class Period>
        [[nodiscard]] std::optional<T> pop_wait_for(std::chrono::duration<Rep, Period> timeout)
        {
            std::unique_lock lock(mutex_);
            std::optional<T> ret = std::nullopt;
            can_consume_.wait_for(lock, timeout, [&] { return closed() || (ret = try_pop()).has_value(); });
            return ret;
        }
    };
}
