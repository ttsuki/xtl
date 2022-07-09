/// @file
/// @brief  xtl::unique_handle - unique_ptr with type-erased deleter
/// @author ttsuki

#pragma once
#include "xtl.config.h"

#include "xtl_delegate.h"

namespace
XTL_NAMESPACE
{
    /// unique_ptr with type-erased deleter
    template <class T, std::enable_if_t<std::is_pointer_v<T>>* = nullptr>
    class unique_handle
    {
        std::unique_ptr<std::remove_pointer_t<T>, delegate<void(T)>> handle_;

    public:
        unique_handle(std::nullptr_t = nullptr)
        {
        }

        unique_handle(T handle, delegate<void(T)> deleter)
            : handle_(handle, std::move(deleter))
        {
        }

        T get() const noexcept { return handle_.get(); }

        T release() noexcept { return handle_.release(); }

        void reset(T p = T()) noexcept { return handle_.reset(p); }
        template <class U> void reset(U) = delete;

        explicit operator bool() const noexcept { return static_cast<bool>(handle_); }
    };

    template class unique_handle<void*>;
}
