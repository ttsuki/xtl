#include <xtl/xtl.h>

#include <iostream>
#include <memory>

struct check : xtl::debug::copy_move_operation_debug_helper::movable<int>
{
    check(int mark) : xtl::debug::copy_move_operation_debug_helper::movable<int>(mark) {}
    //virtual void hello(int a) & noexcept { std::cout << "& hello " << this << ' ' << mark << " a = " << a << std::endl; }
    virtual void hello(int a) const& noexcept { std::cout << "const & hello " << this << ' ' << mark << " a = " << a << std::endl; }
    //virtual void hello(int a) && noexcept { std::cout << " && hello " << this << ' ' << mark << " a = " << a << std::endl; }
    //virtual void hello(int a) const && noexcept { std::cout << "const && hello " << this << ' ' << mark << " a = " << a << std::endl; }
};

struct check2 : check
{
    check2(int mark) : check(mark) {}
    //void hello(int a) noexcept override { std::cout << "hello2 " << this << ' ' << mark << " a = " << a << std::endl; }
    void hello(int a) const& noexcept override { std::cout << "const & hello2 " << this << ' ' << mark << " a = " << a << std::endl; }
};

struct U
{
    void operator()(int a) noexcept { std::cout << "- " << this << " a = " << a << std::endl; }
    void operator()(int a) const noexcept { std::cout << "const " << this << " a = " << a << std::endl; }
};

template <class R, class... A>
struct Test
{
    template <class P, class F, std::enable_if_t<
        std::is_constructible_v<std::remove_const_t<std::remove_reference_t<P>>, P>&&
        std::is_nothrow_move_constructible_v<std::remove_cv_t<std::remove_reference_t<P>>>&&
        std::is_constructible_v<std::remove_const_t<std::remove_reference_t<F>>, F>&&
        std::is_nothrow_move_constructible_v<std::remove_cv_t<std::remove_reference_t<F>>>&&
        std::is_invocable_r_v<R, F, P, A...>
    >* = nullptr>
    static inline R F(P&& instance_pointer, F&& member_function, A&&... a)
    {
        return [instance_pointer = std::forward<P>(instance_pointer), member_function = std::forward<F>(member_function)](A... a)
        {
            return static_cast<R>(std::invoke(member_function, instance_pointer, std::forward<A>(a)...));
        }(std::forward<A>(a)...);
    }
};

void test_func(int) { }

int main()
{
    std::cout << xtl::timestamp::now().to_localtime_string() << std::endl;

    auto t = [b = std::make_unique<check>(42)](int a) noexcept { std::cout << a << " " << b->mark << std::endl; };

    auto de = xtl::delegate(test_func); // CTAD delegate<void(int)>

    xtl::delegate x = std::move(t); // CTAD delegate<void(int)>
    if (x) x(3);

    xtl::delegate y = xtl::delegate(xtl::delegate(std::move(x))); // CTAD delegate<void(int)>
    if (y) y(4);

    x = [b = std::make_unique<check>(43)](int a) { std::cout << a << " " << b->mark << std::endl; };
    if (x) x(5);
    if (y) y(6);
    y = std::move(x);

    {
        const check x{ 98 };
        Test<void, int>::F(&x, &check::hello, 123);
    }

    {
        auto k = std::make_shared<check2>(63);
        x = xtl::delegate{k, & check2::hello}; // binding pointer
    }

    y = xtl::delegate(std::make_unique<check>(12345), &check::hello);
    if (x) x(7);
    if (y) y(8);

    {
        x = xtl::delegate(std::make_unique<check>(111), &check::hello);
        x(9);
    }

    const U u;
    const std::function<void(int)> zz{u};
    zz(3);

    x = zz;
    x(10);

    //xtl::delegate_detail::deduction_test::delegate_type_deduce_test();
}
