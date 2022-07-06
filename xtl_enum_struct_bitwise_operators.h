/// @file
/// @brief  xtl::enum_struct_bitwise_operators
/// @author ttsuki

#pragma once
#include "xtl.config.h"

#include <type_traits>

// define in global namespace, enable if XTL_has_enum_struct_bitwise_operators is found by ADL
template <class T, std::enable_if_t<decltype(XTL_has_enum_struct_bitwise_operators(std::declval<T>()))::value && std::is_enum_v<T>, nullptr_t>  = nullptr> static inline constexpr auto operator +(T rhs) -> std::underlying_type_t<T> { return static_cast<std::underlying_type_t<T>>(rhs); }
template <class T, std::enable_if_t<decltype(XTL_has_enum_struct_bitwise_operators(std::declval<T>()))::value && std::is_enum_v<T>, nullptr_t>  = nullptr> static inline constexpr T operator |(T lhs, T rhs) { return static_cast<T>(+lhs | +rhs); }
template <class T, std::enable_if_t<decltype(XTL_has_enum_struct_bitwise_operators(std::declval<T>()))::value && std::is_enum_v<T>, nullptr_t>  = nullptr> static inline constexpr T operator &(T lhs, T rhs) { return static_cast<T>(+lhs & +rhs); }
template <class T, std::enable_if_t<decltype(XTL_has_enum_struct_bitwise_operators(std::declval<T>()))::value && std::is_enum_v<T>, nullptr_t>  = nullptr> static inline constexpr T operator ^(T lhs, T rhs) { return static_cast<T>(+lhs ^ +rhs); }
template <class T, std::enable_if_t<decltype(XTL_has_enum_struct_bitwise_operators(std::declval<T>()))::value && std::is_enum_v<T>, nullptr_t>  = nullptr> static inline constexpr T& operator |=(T& lhs, T rhs) { return lhs = lhs | rhs; }
template <class T, std::enable_if_t<decltype(XTL_has_enum_struct_bitwise_operators(std::declval<T>()))::value && std::is_enum_v<T>, nullptr_t>  = nullptr> static inline constexpr T& operator &=(T& lhs, T rhs) { return lhs = lhs & rhs; }
template <class T, std::enable_if_t<decltype(XTL_has_enum_struct_bitwise_operators(std::declval<T>()))::value && std::is_enum_v<T>, nullptr_t>  = nullptr> static inline constexpr T& operator ^=(T& lhs, T rhs) { return lhs = lhs ^ rhs; }

// define ADL XTL_has_enum_struct_bitwise_operators
#define XTL_enable_enum_struct_bitwise_operators(T) static std::true_type XTL_has_enum_struct_bitwise_operators(T); // for ADL
