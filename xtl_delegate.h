/// @file
/// @brief  xtl delegate - a move only function<...> implementation.
/// @author ttsuki

#pragma once

#include <cassert>
#include <type_traits>
#include <utility>

#include <memory>
#include <functional>

#include "xtl_small_object_optimization.h"

namespace xtl
{
    template <typename T>
    class delegate;

    template <class R, class... A>
    class delegate<R(A...)>
    {
    public:
        struct alignment
        {
            alignas(std::max_align_t) std::byte data[sizeof(void*) * 8];
        };

    private:
        struct soo : small_object_optimization<alignment>
        {
            using base = small_object_optimization<alignment>;
            using memory = typename base::memory;
            using move_constructor_function = typename base::move_constructor_function;
            using destructor_function = typename base::destructor_function;

            using invoke_function = R(*)(memory&, A...);

            template <class T>
            static R invoke(memory& m, A... args)
            {
                 return (*base::template pointer<T>(m))(std::forward<A>(args)...);
            }

            struct vtable
            {
                invoke_function invoke{};
                move_constructor_function move_ctor{};
                destructor_function dtor{};
            };

            template <class T>
            static constexpr inline vtable vtable_for = {
                &soo::invoke<T>,
                &base::template move_construct<T>,
                &base::template destruct<T>
            };
        };

        mutable typename soo::memory memory_{};
        const typename soo::vtable* vtable_{};

        // emplace
        template <class T>
        void emplace(T&& args)
        {
            using U = std::remove_cv_t<std::remove_reference_t<T>>;

            this->reset();
            soo::template construct<U>(this->memory_, std::forward<T>(args));
            this->vtable_ = &soo::template vtable_for<U>;
        }

        // move from
        void move_assign(delegate&& other)
        {
            if (other.vtable_)
            {
                this->vtable_ = std::exchange(other.vtable_, nullptr);
                this->vtable_->move_ctor(this->memory_, std::move(other.memory_));
            }
        }

        // destruct
        void reset() noexcept
        {
            if (this->vtable_)
            {
                this->vtable_->dtor(this->memory_);
                this->vtable_ = nullptr;
            }
        }

    public:
        // empty delegate
        delegate() = default;

        // from nullptr (empty)
        delegate(std::nullptr_t) { }

        // do not copy
        delegate(const delegate&) = delete;

        // do not copy assign
        delegate& operator=(const delegate&) = delete;

        // move
        delegate(delegate&& other) noexcept
        {
            this->move_assign(std::move(other));
        }

        // move assign
        delegate& operator=(delegate&& other) noexcept
        {
            if (std::addressof(other) != this)
            {
                this->reset();
                this->move_assign(std::move(other));
            }
            return *this;
        }

        // from functor
        template <class Functor, std::enable_if_t<
                      std::is_class_v<std::decay_t<Functor>> &&
                      std::is_nothrow_move_constructible_v<std::decay_t<Functor>> &&
                      std::is_same_v<R, std::invoke_result_t<Functor, A...>> &&
                      std::is_constructible_v<std::remove_cv_t<std::remove_reference_t<Functor>>, Functor>
                  >* = nullptr>
        delegate(Functor&& f)
        {
            this->emplace(std::forward<Functor>(f));
        }

        // from function pointer
        delegate(R (*function_pointer)(A...))
        {
            this->emplace<R(*)(A...)>(static_cast<R(*)(A...)>(function_pointer));
        }

#if defined(_MSC_VER) && defined(_M_IX86)
        delegate(R (__stdcall *function_pointer)(A...)) { this->emplace<R(__stdcall*)(A...)>(static_cast<R(__stdcall*)(A...)>(function_pointer)); }
        delegate(R (__thiscall *function_pointer)(A...)) { this->emplace<R(__thiscall*)(A...)>(static_cast<R(__thiscall*)(A...)>(function_pointer)); }
#endif

        // from member function pointer
        template <class C>
        delegate(C* instance_pointer, R (C::*member_function)(A...))
            : delegate([instance_pointer, member_function](A... a)
            {
                return std::invoke(member_function, *instance_pointer, std::forward<A>(a)...);
            }) { }

        // from member function pointer
        template <class C>
        delegate(const C* instance_pointer, R (C::*member_function)(A...) const)
            : delegate([instance_pointer, member_function](A... a)
            {
                return std::invoke(member_function, *instance_pointer, std::forward<A>(a)...);
            }) { }

        // from member function pointer (with instance pointer-like type)
        template <class P, class K, class C = std::remove_reference_t<decltype(*std::declval<P>())>, std::enable_if_t<
                      std::is_class_v<std::remove_cv_t<std::remove_reference_t<P>>> &&
                      std::is_constructible_v<std::remove_cv_t<std::remove_reference_t<P>>, P> &&
                      std::is_const_v<C> && std::is_invocable_r_v<R, R(K::*)(A...) const, decltype(*std::declval<P>()), A...>
                  >* = nullptr>
        delegate(P&& instance_pointer, R (K::*member_function)(A...) const)
            : delegate([instance_pointer = std::forward<P>(instance_pointer), member_function](A... a)
            {
                return std::invoke(member_function, *instance_pointer, std::forward<A>(a)...);
            }) { }

        // from member function pointer (with instance pointer-like type)
        template <class P, class K, class C = std::remove_reference_t<decltype(*std::declval<P>())>, std::enable_if_t<
                      std::is_class_v<std::remove_cv_t<std::remove_reference_t<P>>> &&
                      std::is_constructible_v<std::remove_cv_t<std::remove_reference_t<P>>, P> &&
                      !std::is_const_v<C> && std::is_invocable_r_v<R, R (K::*)(A...) const, decltype(*std::declval<P>()), A...>
                  >* = nullptr>
        delegate(P&& instance_pointer, R (K::*member_function)(A...) const)
            : delegate([instance_pointer = std::forward<P>(instance_pointer), member_function](A... a)
            {
                return std::invoke(member_function, *instance_pointer, std::forward<A>(a)...);
            }) { }

        // from member function pointer (with instance pointer-like type)
        template <class P, class K, class C = std::remove_reference_t<decltype(*std::declval<P>())>, std::enable_if_t<
                      std::is_class_v<std::remove_cv_t<std::remove_reference_t<P>>> &&
                      std::is_constructible_v<std::remove_cv_t<std::remove_reference_t<P>>, P> &&
                      !std::is_const_v<C> && std::is_invocable_r_v<R, R (K::*)(A...), decltype(*std::declval<P>()), A...>
                  >* = nullptr>
        delegate(P&& instance_pointer, R (K::*member_function)(A...))
            : delegate([instance_pointer = std::forward<P>(instance_pointer), member_function](A... a)
            {
                return std::invoke(member_function, *instance_pointer, std::forward<A>(a)...);
            }) { }

        // destruct
        ~delegate()
        {
            this->reset();
        }

        // can be invoked
        explicit operator bool() const noexcept
        {
            return this->vtable_;
        }

        // invoke
        R operator()(A... args) const
        {
            if (this->vtable_) return this->vtable_->invoke(this->memory_, std::forward<A>(args)...);
            else throw std::bad_function_call();
        }
    };

    // Deduction guides for delegate
    // @formatter:off

    // from function pointers
    template <         class R, class... A> delegate(R (          *)(A...)) -> delegate<R(A...)>;
#if defined(_MSC_VER) && defined(_M_IX86)
    template <         class R, class... A> delegate(R (__stdcall *)(A...)) -> delegate<R(A...)>;
    template <         class R, class... A> delegate(R (__thiscall*)(A...)) -> delegate<R(A...)>;
#endif

    // from member function pointers with binding instance pointer
    template <         class C, class R, class... A> delegate(      C*, R(C::*)(A...)      ) -> delegate<R(A...)>;
    template <         class C, class R, class... A> delegate(const C*, R(C::*)(A...) const) -> delegate<R(A...)>;
    template <class P, class C, class R, class... A> delegate(     P&&, R(C::*)(A...)      ) -> delegate<R(A...)>;
    template <class P, class C, class R, class... A> delegate(     P&&, R(C::*)(A...) const) -> delegate<R(A...)>;

    // from others
    namespace delegate_detail
    {
        template <class F> struct function_type_deduction;
        template <class C, class R, class... A> struct function_type_deduction<R(C::*)(A...)                > { using type = R(A...); };
        template <class C, class R, class... A> struct function_type_deduction<R(C::*)(A...) const          > { using type = R(A...); };
        template <class C, class R, class... A> struct function_type_deduction<R(C::*)(A...)       noexcept > { using type = R(A...); };
        template <class C, class R, class... A> struct function_type_deduction<R(C::*)(A...) const noexcept > { using type = R(A...); };
    }
    template <class F> delegate(F) -> delegate<typename delegate_detail::function_type_deduction<decltype(&F::operator())>::type>;

    // @formatter:on

    template <class T> using any_invokable = delegate<T>;

#if 0 // type deduction guide test
    namespace delegate_detail::deduction_test
    {
        static void a_nonmember_function() noexcept { }

        static void delegate_type_deduce_test()
        {
            {
                delegate a = a_nonmember_function;
                a();
            }

            {
                struct F
                {
                    void operator ()(int) { }
                    void operator ()(int) const { }
                };

                F i;
                delegate<void(int)> a{static_cast<const F>(F())};
                static_assert(std::is_same_v<decltype(a), delegate<void(int)>>);

                delegate<void(int)> b{&i, &F::operator()};
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
                static_assert(std::is_same_v<decltype(a), delegate<int(void*, long long)>>);
                delegate b{&i, &F::operator()};
                static_assert(std::is_same_v<decltype(b), delegate<int(void*, long long)>>);
                delegate c{std::make_unique<F>(), &F::operator()};
                static_assert(std::is_same_v<decltype(c), delegate<int(void*, long long)>>);
            }

            {
                delegate a = [something_uncopyable = std::unique_ptr<nullptr_t>()](void*) -> int { return 42; };
                static_assert(std::is_same_v<decltype(a), delegate<int(void*)>>);
            }
        }
    }
#endif
}
