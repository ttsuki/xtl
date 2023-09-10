/// @file
/// @brief  xtl::enum_struct_bitwise_operators
/// @author (C) 2022 ttsuki
/// Distributed under the Boost Software License, Version 1.0.

#pragma once

#include <type_traits>

// define ADL XTL_has_enum_struct_bitwise_operators
#define XTL_enable_enum_struct_bitwise_operators(T) static inline std::true_type xtl_enum_struct_bitwise_operators_enabled(T) { return std::true_type{}; } // for ADL

// define in global namespace, enable if XTL_has_enum_struct_bitwise_operators is found by ADL
template <class T, std::enable_if_t<decltype(xtl_enum_struct_bitwise_operators_enabled(std::declval<T>()))::value && std::is_enum_v<T>, std::nullptr_t>  = nullptr> static inline constexpr auto operator +(T rhs) -> std::underlying_type_t<T> { return static_cast<std::underlying_type_t<T>>(rhs); } // std::to_underlying
template <class T, std::enable_if_t<decltype(xtl_enum_struct_bitwise_operators_enabled(std::declval<T>()))::value && std::is_enum_v<T>, std::nullptr_t>  = nullptr> static inline constexpr auto operator !(T rhs) -> std::underlying_type_t<T> { return !(+rhs); }
template <class T, std::enable_if_t<decltype(xtl_enum_struct_bitwise_operators_enabled(std::declval<T>()))::value && std::is_enum_v<T>, std::nullptr_t>  = nullptr> static inline constexpr T operator ~(T rhs) { return static_cast<T>(~(+rhs)); }
template <class T, std::enable_if_t<decltype(xtl_enum_struct_bitwise_operators_enabled(std::declval<T>()))::value && std::is_enum_v<T>, std::nullptr_t>  = nullptr> static inline constexpr T operator |(T lhs, T rhs) { return static_cast<T>(+lhs | +rhs); }
template <class T, std::enable_if_t<decltype(xtl_enum_struct_bitwise_operators_enabled(std::declval<T>()))::value && std::is_enum_v<T>, std::nullptr_t>  = nullptr> static inline constexpr T operator &(T lhs, T rhs) { return static_cast<T>(+lhs & +rhs); }
template <class T, std::enable_if_t<decltype(xtl_enum_struct_bitwise_operators_enabled(std::declval<T>()))::value && std::is_enum_v<T>, std::nullptr_t>  = nullptr> static inline constexpr T operator ^(T lhs, T rhs) { return static_cast<T>(+lhs ^ +rhs); }
template <class T, std::enable_if_t<decltype(xtl_enum_struct_bitwise_operators_enabled(std::declval<T>()))::value && std::is_enum_v<T>, std::nullptr_t>  = nullptr> static inline constexpr T& operator |=(T& lhs, T rhs) { return lhs = lhs | rhs; }
template <class T, std::enable_if_t<decltype(xtl_enum_struct_bitwise_operators_enabled(std::declval<T>()))::value && std::is_enum_v<T>, std::nullptr_t>  = nullptr> static inline constexpr T& operator &=(T& lhs, T rhs) { return lhs = lhs & rhs; }
template <class T, std::enable_if_t<decltype(xtl_enum_struct_bitwise_operators_enabled(std::declval<T>()))::value && std::is_enum_v<T>, std::nullptr_t>  = nullptr> static inline constexpr T& operator ^=(T& lhs, T rhs) { return lhs = lhs ^ rhs; }
