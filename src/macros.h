//
// Created by Simon Cros on 25/01/2026.
//

#ifndef INC_42RUN_MACROS_H
#define INC_42RUN_MACROS_H

#include <cassert>
#include <type_traits>

#define MAKE_FLAG_ENUM(Enum) \
    inline Enum operator~(const Enum a) { return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(a)); } \
    inline Enum operator|(const Enum a, const Enum b) { return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(a) | static_cast<std::underlying_type_t<Enum>>(b)); } \
    inline Enum operator&(const Enum a, const Enum b) { return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(a) & static_cast<std::underlying_type_t<Enum>>(b)); } \
    inline Enum operator^(const Enum a, const Enum b) { return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(a) ^ static_cast<std::underlying_type_t<Enum>>(b)); } \
    inline Enum& operator|=(Enum& a, const Enum b) { return reinterpret_cast<Enum&>(reinterpret_cast<std::underlying_type_t<Enum>&>(a) |= static_cast<std::underlying_type_t<Enum>>(b)); } \
    inline Enum& operator&=(Enum& a, const Enum b) { return reinterpret_cast<Enum&>(reinterpret_cast<std::underlying_type_t<Enum>&>(a) &= static_cast<std::underlying_type_t<Enum>>(b)); } \
    inline Enum& operator^=(Enum& a, const Enum b) { return reinterpret_cast<Enum&>(reinterpret_cast<std::underlying_type_t<Enum>&>(a) ^= static_cast<std::underlying_type_t<Enum>>(b)); }

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#define UNIQUE_NAME(prefix) CONCAT(prefix, __LINE__)

#define TRY(expression) \
    do { \
        if (auto UNIQUE_NAME(_expected_) = (expression); !UNIQUE_NAME(_expected_)) \
        { \
            return std::unexpected(std::move(UNIQUE_NAME(_expected_)).error()); \
        } \
    } while (0)

#define TRY_V(type, variable, expression) \
    auto UNIQUE_NAME(_expected_) = (expression); \
    if (!UNIQUE_NAME(_expected_)) \
    { \
        return std::unexpected(std::move(UNIQUE_NAME(_expected_)).error()); \
    } \
    type variable = std::move(UNIQUE_NAME(_expected_)).value()

#endif //INC_42RUN_MACROS_H