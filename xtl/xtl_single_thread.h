/// @file
/// @brief  xtl::single_thread
/// @author (C) 2023 ttsuki
/// Distributed under the Boost Software License, Version 1.0.

#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <future>

namespace xtl
{
    class single_thread
    {
        std::mutex mutex_{};
        std::condition_variable cv_{};
        std::atomic_flag running_{};
        std::atomic_flag sleeping_{};

        void* ctx_{};
        void (*func_)(void* ctx){};
        std::thread thread_{};

    public:
        single_thread()
        {
            running_.test_and_set();
            sleeping_.test_and_set();
            thread_ = std::thread{
                [this]
                {
                    std::unique_lock l(mutex_);
                    while (true)
                    {
                        cv_.wait(l, [&] { return !sleeping_.test_and_set(); });
                        if (!running_.test_and_set()) break;
                        if (func_) { func_(ctx_); }
                    }
                }
            };
        }

        single_thread(const single_thread& other) = delete;
        single_thread(single_thread&& other) noexcept = delete;
        single_thread& operator=(const single_thread& other) = delete;
        single_thread& operator=(single_thread&& other) noexcept = delete;

        ~single_thread()
        {
            running_.clear();
            sleeping_.clear();
            cv_.notify_one();
            thread_.join();
        }

        template <class F, std::enable_if_t<std::is_invocable_v<F>>* = nullptr>
        auto invoke(F&& func)
        {
            using R = std::invoke_result_t<F>;

            std::promise<R> promise;
            struct ctx_t
            {
                std::remove_reference_t<F>* f;
                std::promise<R>* r;
            } ctx{&func, &promise};

            {
                std::unique_lock lock(mutex_);
                ctx_ = &ctx;
                func_ = [](void* p)
                {
                    auto [f, r] = *static_cast<ctx_t*>(p);
                    try
                    {
                        if constexpr (!std::is_same_v<R, void>)
                        {
                            (*r).set_value((*f)());
                        }
                        else
                        {
                            (*f)(), (*r).set_value();
                        }
                    }
                    catch (...)
                    {
                        (*r).set_exception(std::current_exception());
                    }
                };
                sleeping_.clear();
                cv_.notify_one();
            }

            return promise.get_future().get();
        }
    };
}
