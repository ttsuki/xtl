/// @file
/// @brief  xtl delegate - a move only function<...> implementation.
/// @author (C) 2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <cassert>
#include <type_traits>
#include <utility>

#include <new>
#include <array>
#include <memory>
#include <variant>
#include <functional>

namespace
XTL_NAMESPACE
{
    template <typename T> class delegate;

    template <class R, class ...A>
    class delegate<R (A ...)>
    {
        using alignment_t = void*;
        using sso_memory = std::array<alignment_t, 4>;
        using heap_memory = std::unique_ptr<alignment_t[]>;
        std::variant<nullptr_t, sso_memory, heap_memory> memory_ = nullptr;
        R (*invoke_)(delegate*, A ...) = nullptr;
        void (*destruct_)(delegate*) = nullptr;
        void (*move_)(delegate*, delegate*) = nullptr;

        template <class T> bool holds() const noexcept { return std::holds_alternative<T>(memory_); }

    public:
        delegate() = default;

        delegate(nullptr_t)
        {
        }

        template <class Functor, std::enable_if_t<
                      std::is_class_v<Functor> &&
                      std::is_nothrow_move_constructible_v<Functor> &&
                      std::is_same_v<R, std::invoke_result_t<Functor, A...>>
                  > * = nullptr>
        delegate(Functor f)
        {
            constexpr bool sso = sizeof(Functor) <= sizeof(sso_memory);
            using memory_type = std::conditional_t<sso, sso_memory, heap_memory>;

            // allocate memory
            if constexpr (sso) memory_ = sso_memory{};
            else memory_ = std::make_unique<alignment_t[]>((sizeof(f) + sizeof(alignment_t) - 1) / sizeof(alignment_t));

            struct helper
            {
                static Functor* functor_pointer(delegate* this_)
                {
                    if constexpr (sso) return reinterpret_cast<Functor*>(std::get<sso_memory>(this_->memory_).data());
                    else return reinterpret_cast<Functor*>(std::get<heap_memory>(this_->memory_).get());
                }
            };

            // move construct lambda with placement new.
            new(helper::functor_pointer(this)) Functor(std::move(f));

            // invoker: operator()
            invoke_ = [](delegate* this_, A ...a) -> R { return helper::functor_pointer(this_)->operator()(std::forward<A>(a)...); };

            // destructor: ~this();
            destruct_ = [](delegate* this_) { helper::functor_pointer(this_)->~Functor(); };

            // move operator: this = std::move(other);
            move_ = [](delegate* this_, delegate* other)
            {
                assert(this_->holds<nullptr_t>());

                if (other->holds<nullptr_t>())
                {
                    // do nothing
                }
                else if (other->holds<sso_memory>())
                {
                    // move construct with placement new, and destruct other.
                    this_->memory_ = sso_memory{};
                    new(helper::functor_pointer(this_)) Functor(std::move(*helper::functor_pointer(other)));
                    helper::functor_pointer(other)->~Functor();
                    other->memory_ = nullptr;
                }
                else if (other->holds<heap_memory>())
                {
                    // move unique_ptr, and destruct other.
                    this_->memory_ = std::move(std::get<heap_memory>(other->memory_));
                    other->memory_ = nullptr;
                }

                // move traits
                this_->invoke_ = std::exchange(other->invoke_, nullptr);
                this_->destruct_ = std::exchange(other->destruct_, nullptr);
                this_->move_ = std::exchange(other->move_, nullptr);
            };
        }

        delegate(R (*function_pointer)(A ...)) : delegate([function_pointer](A ... a) { return function_pointer(std::forward<A>(a)...); }) { }
        template <class T> delegate(T* instance, R (T::*member_function)(A ...)) : delegate([instance, member_function](A ...a) { return (instance->*member_function)(std::forward<A>(a)...); }) { }
        template <class T> delegate(const T* instance, R (T::*member_function)(A ...) const) : delegate([instance, member_function](A ...a) { return (instance->*member_function)(std::forward<A>(a)...); }) { }
#if defined(_MSC_VER) && defined(_M_IX86)
        delegate(R (__stdcall  *function_pointer)(A ...)) : delegate([function_pointer](A ... a) { return function_pointer(std::forward<A>(a)...); }) { }
        delegate(R (__thiscall *function_pointer)(A ...)) : delegate([function_pointer](A ... a) { return function_pointer(std::forward<A>(a)...); }) { }
#endif

        delegate(const delegate&) = delete;

        delegate(delegate&& other) noexcept
        {
            if (other.move_)
                other.move_(this, &other);
        }

        delegate& operator=(const delegate&) = delete;

        delegate& operator=(delegate&& other) noexcept
        {
            if (std::addressof(other) != this)
            {
                if (this->destruct_) this->destruct_(this);
                this->memory_ = nullptr;
                this->invoke_ = nullptr;
                this->move_ = nullptr;
                this->destruct_ = nullptr;

                if (other.move_)
                    other.move_(this, &other);
            }

            return *this;
        }

        ~delegate()
        {
            if (this->destruct_) this->destruct_(this);
            this->memory_ = nullptr;
            this->invoke_ = nullptr;
            this->move_ = nullptr;
            this->destruct_ = nullptr;
        }

        R operator()(A ...args) const
        {
            if (invoke_) return invoke_(const_cast<delegate*>(this), std::forward<A>(args)...);
            else throw std::bad_function_call();
        }

        explicit operator bool() const noexcept { return invoke_; }
    };

    namespace delegate_detail
    {
        // @formatter:off
        template <class F> struct function_type_deduction;
        template <         class R, class... A> struct function_type_deduction<R(   *)(A ...)                > { using type = R(A ...); };
        template <class C, class R, class... A> struct function_type_deduction<R(C::*)(A ...)                > { using type = R(A ...); };
        template <class C, class R, class... A> struct function_type_deduction<R(C::*)(A ...) const          > { using type = R(A ...); };
        template <class C, class R, class... A> struct function_type_deduction<R(C::*)(A ...)       noexcept > { using type = R(A ...); };
        template <class C, class R, class... A> struct function_type_deduction<R(C::*)(A ...) const noexcept > { using type = R(A ...); };
        // @formatter:on
    }

    template <class R, class ...A> delegate(R (*)(A ...)) -> delegate<R(A ...)>;
    template <class C, class R, class ...A> delegate(C*, R (C::*)(A ...)) -> delegate<R(A ...)>;
    template <class C, class R, class ...A> delegate(const C*, R (C::*)(A ...) const) -> delegate<R(A ...)>;
    template <class F> delegate(F) -> delegate<typename delegate_detail::function_type_deduction<decltype(&F::operator())>::type>;
#if defined(_MSC_VER) && defined(_M_IX86)
    template <class R, class ...A> delegate(R (__stdcall  *)(A ...)) -> delegate<R(A ...)>;
    template <class R, class ...A> delegate(R (__thiscall *)(A ...)) -> delegate<R(A ...)>;
#endif

#if 0 // type deduction guide test
    namespace delegate_detail
    {
        static void test() noexcept
        {
        }

        static void delegate_type_deduce_test()
        {
            {
                delegate a = test;
            }

            {
                struct F
                {
                    void operator ()(int)
                    {
                    }
                };
                F i;
                delegate a{F()};
                delegate b{&i, &F::operator()};
                static_assert(std::is_same_v<decltype(a), delegate<void(int)>>);
                static_assert(std::is_same_v<decltype(b), delegate<void(int)>>);
            }

            {
                struct F
                {
                    int operator ()(void*, long long) const noexcept
                    {
                        return 42;
                    }
                };
                F i;
                delegate a(i);
                delegate b{&i, &F::operator()};
                static_assert(std::is_same_v<decltype(a), delegate<int(void*, long long)>>);
                static_assert(std::is_same_v<decltype(b), delegate<int(void*, long long)>>);
            }

            {
                delegate a = [something_uncopyable = std::unique_ptr<nullptr_t>()](void*) -> int { return 42; };
                static_assert(std::is_same_v<decltype(a), delegate<int(void*)>>);
            }
        }
    }
#endif
}
