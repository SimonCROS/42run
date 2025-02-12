//
// Created by Simon Cros on 02/01/2025.
//

#ifndef EXPECTED_HPP
#define EXPECTED_HPP

#include <version>

#if __cpp_lib_expected >= 202211L

#include <expected>

using Unexpect = std::unexpect_t;

inline constexpr Unexpect unexpect = std::unexpect;

template<class T, class E>
using Expected = std::expected<T, E>;

template<class E>
using Unexpected = std::unexpected<E>;

#else

#include <variant>
#include <concepts>

#ifndef LIBFTPP_EXPECTED
#define LIBFTPP_EXPECTED
#endif

struct Unexpect
{
    explicit Unexpect() = default;
};

inline constexpr Unexpect unexpect{};

template <class E>
class Unexpected
{
private:
    E m_error;

public:
    constexpr Unexpected()
        requires
        std::default_initializable<E>
        : m_error()
    {
    }

    constexpr Unexpected(const Unexpected& other) = default;

    constexpr Unexpected(Unexpected&& other) = default;

    template <class Err = E>
    constexpr
    explicit
    Unexpected(Err&& e)
        requires (!std::same_as<std::remove_cvref_t<Err>, std::in_place_t>)
        && (!std::same_as<std::remove_cvref_t<Err>, Unexpected>)
        && std::constructible_from<E, Err>
        : m_error(std::forward<Err>(e))
    {
    }

    template <class... Args>
    constexpr explicit Unexpected(std::in_place_t, Args&&... args)
        requires std::constructible_from<E, Args...>
        : m_error(std::forward<Args>(args)...)
    {
    }

    template <class U, class... Args>
    constexpr
    explicit
    Unexpected(std::in_place_t, std::initializer_list<U> il, Args&&... args)
        requires std::constructible_from<E, std::initializer_list<U>&, Args...>
        : m_error(il, std::forward<Args>(args)...)
    {
    }

    [[nodiscard]] constexpr auto error() & -> E&
    {
        return m_error;
    }

    [[nodiscard]] constexpr auto error() const & -> const E&
    {
        return m_error;
    }

    [[nodiscard]] constexpr auto error() && -> E&&
    {
        return std::move(m_error);
    }

    [[nodiscard]] constexpr auto error() const && -> const E&&
    {
        return std::move(m_error);
    }
};

template <class E>
Unexpected(E) -> Unexpected<E>;

template <class U>
static constexpr bool is_unexpected = false;

template <class U>
static constexpr bool is_unexpected<Unexpected<U>> = true;

template <class T, class E>
class Expected
{
public:
    using value_type = T;
    using error_type = E;
    using unexpected_type = Unexpected<E>;

    template <class U>
    using rebind = Expected<U, error_type>;

private:
    template <class U, class G>
    friend class Expected;

    using variant_type = std::variant<std::monostate, T, E>;

    static constexpr std::size_t t_index = 1;
    static constexpr std::size_t e_index = 2;

    // Declared above, for compatibility with compilers not following CWG 727
    //
    // template <class U>
    // static constexpr bool is_unexpected = false;
    //
    // template <class U>
    // static constexpr bool is_unexpected<Unexpected<U>> = true;

    variant_type m_variant;

public:
    constexpr Expected()
        requires
        std::default_initializable<variant_type>
        : m_variant(std::in_place_index<t_index>)
    {
    }

    Expected(const Expected& other) = default;

    constexpr Expected(const Expected& other)
        requires std::copy_constructible<variant_type> && (!std::is_trivially_copy_constructible_v<variant_type>)
        : m_variant(other.m_variant)
    {
    }

    constexpr Expected(Expected&& other)
        noexcept(std::is_nothrow_move_constructible_v<variant_type>)
        requires std::move_constructible<variant_type> && (!std::is_trivially_move_constructible_v<variant_type>)\
        : m_variant(std::move(other.m_variant))
    {
    }

    template <class U, class G>
    constexpr
    explicit(!std::is_convertible_v<const U&, T> || !std::is_convertible_v<const G&, E>)
    Expected(const Expected<U, G>& other)
        requires std::constructible_from<T, const U&> && std::constructible_from<E, const G&>
        && (!std::constructible_from<T, Expected<U, G>&>)
        && (!std::constructible_from<T, Expected<U, G>>)
        && (!std::constructible_from<T, const Expected<U, G>&>)
        && (!std::constructible_from<T, const Expected<U, G>>)
        && (!std::convertible_to<Expected<U, G>&, T>)
        && (!std::convertible_to<Expected<U, G>, T>)
        && (!std::convertible_to<const Expected<U, G>&, T>)
        && (!std::convertible_to<const Expected<U, G>, T>)
        && (!std::constructible_from<unexpected_type, Expected<U, G>&>)
        && (!std::constructible_from<unexpected_type, Expected<U, G>>)
        && (!std::constructible_from<unexpected_type, const Expected<U, G>&>)
        && (!std::constructible_from<unexpected_type, const Expected<U, G>>)
        : m_variant()
    {
        if (other.has_value())
            m_variant.template emplace<t_index>(std::forward<const U&>(other.value()));
        else
            m_variant.template emplace<e_index>(std::forward<const G&>(other.error()));
    }

    template <class U, class G>
    constexpr
    explicit(!std::is_convertible_v<U, T> || !std::is_convertible_v<G, E>)
    Expected(Expected<U, G>&& other)
        requires std::constructible_from<T, U> && std::constructible_from<E, G>
        && (!std::constructible_from<T, Expected<U, G>&>)
        && (!std::constructible_from<T, Expected<U, G>>)
        && (!std::constructible_from<T, const Expected<U, G>&>)
        && (!std::constructible_from<T, const Expected<U, G>>)
        && (!std::convertible_to<Expected<U, G>&, T>)
        && (!std::convertible_to<Expected<U, G>, T>)
        && (!std::convertible_to<const Expected<U, G>&, T>)
        && (!std::convertible_to<const Expected<U, G>, T>)
        && (!std::constructible_from<unexpected_type, Expected<U, G>&>)
        && (!std::constructible_from<unexpected_type, Expected<U, G>>)
        && (!std::constructible_from<unexpected_type, const Expected<U, G>&>)
        && (!std::constructible_from<unexpected_type, const Expected<U, G>>)
        : m_variant()
    {
        if (other.has_value())
        {
            m_variant.template emplace<t_index>(std::forward<U>(other.value()));
        }
        else
            m_variant.template emplace<e_index>(std::forward<G>(other.error()));
    }

    template <class U = T>
    constexpr
    explicit(!std::is_convertible_v<U, T>)
    Expected(U&& v)
        requires (!std::same_as<std::remove_cvref_t<U>, std::in_place_t>)
        && (!std::same_as<Expected<T, E>, std::remove_cvref_t<U>>)
        && (!is_unexpected<std::remove_cvref_t<U>>)
        && std::constructible_from<T, U>
        : m_variant(std::in_place_index<t_index>, std::forward<U>(v))
    {
    }

    template <class G>
    constexpr
    explicit(!std::is_convertible_v<const G&, E>)
    Expected(const Unexpected<G>& e)
        requires std::constructible_from<E, const G&>
        : m_variant(std::in_place_index<e_index>, std::forward<const G&>(e.error()))
    {
    }

    template <class G>
    constexpr explicit(!std::is_convertible_v<G, E>)
    Expected(Unexpected<G>&& e)
        requires std::constructible_from<E, G>
        : m_variant(std::in_place_index<e_index>, std::forward<G>(e.error()))
    {
    }

    template <class... Args>
    constexpr explicit Expected(std::in_place_t, Args&&... args)
        requires std::constructible_from<T, Args...>
        : m_variant(std::in_place_index<t_index>, std::forward<Args>(args)...)
    {
    }

    template <class U, class... Args>
    constexpr
    explicit
    Expected(std::in_place_t, std::initializer_list<U> il, Args&&... args)
        requires std::constructible_from<T, std::initializer_list<U>&, Args...>
        : m_variant(std::in_place_index<t_index>, il, std::forward<Args>(args)...)
    {
    }

    template <class... Args>
    constexpr
    explicit
    Expected(Unexpect, Args&&... args)
        requires std::constructible_from<E, Args...>
        : m_variant(std::in_place_index<e_index>, std::forward<Args>(args)...)
    {
    }

    template <class U, class... Args>
    constexpr
    explicit
    Expected(Unexpect, std::initializer_list<U> il, Args&&... args)
        requires std::constructible_from<E, std::initializer_list<U>&, Args...>
        : m_variant(std::in_place_index<e_index>, il, std::forward<Args>(args)...)
    {
    }

    constexpr auto operator=(const Expected& other) -> Expected& = default;

    constexpr auto operator=(Expected&& other) -> Expected& = default;

    template <class U = T>
    constexpr auto operator=(U&& v) -> Expected& requires (!std::same_as<Expected, std::remove_cvref_t<U>>)
        && (!is_unexpected<std::remove_cvref_t<U>>)
        && std::constructible_from<T, U> && std::assignable_from<T&, U>
        && (std::is_nothrow_constructible_v<T, U>
            || std::is_nothrow_move_constructible_v<T>
            || std::is_nothrow_move_constructible_v<E>)
    {
        m_variant.template emplace<t_index>(std::forward<U>(v));
        return *this;
    }

    template <class G>
    constexpr auto operator=(const Unexpected<G>& e) -> Expected& requires std::constructible_from<E, const G&>
        && std::assignable_from<E&, const G&>
        && (std::is_nothrow_constructible_v<E, const G&> || std::is_nothrow_move_constructible_v<T> ||
            std::is_nothrow_move_constructible_v<E>)
    {
        m_variant.template emplace<e_index>(std::forward<const G&>(e.error()));
        return *this;
    }

    template <class G>
    constexpr auto operator=(Unexpected<G>&& e) -> Expected& requires std::constructible_from<E, G>
        && std::assignable_from<E&, G>
        && (std::is_nothrow_constructible_v<E, G> || std::is_nothrow_move_constructible_v<T> ||
            std::is_nothrow_move_constructible_v<E>)
    {
        m_variant.template emplace<e_index>(std::forward<G>(e.error()));
        return *this;
    }

    [[nodiscard]] constexpr auto has_value() const noexcept -> bool
    {
        return m_variant.index() == t_index;
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }

    [[nodiscard]] constexpr auto value() & -> T&
    {
        return std::get<t_index>(m_variant);
    }

    [[nodiscard]] constexpr auto value() const & -> const T&
    {
        return std::get<t_index>(m_variant);
    }

    [[nodiscard]] constexpr auto value() && -> T&&
    {
        return std::get<t_index>(std::move(m_variant));
    }

    [[nodiscard]] constexpr auto value() const && -> const T&&
    {
        return std::get<t_index>(std::move(m_variant));
    }

    [[nodiscard]] constexpr auto error() & -> E&
    {
        return std::get<e_index>(m_variant);
    }

    [[nodiscard]] constexpr auto error() const & -> const E&
    {
        return std::get<e_index>(m_variant);
    }

    [[nodiscard]] constexpr auto error() && -> E&&
    {
        return std::get<e_index>(std::move(m_variant));
    }

    [[nodiscard]] constexpr auto error() const && -> const E&&
    {
        return std::get<e_index>(std::move(m_variant));
    }

    [[nodiscard]] constexpr auto operator->() const noexcept -> const T*
    {
        return &std::get<t_index>(m_variant);
    }

    [[nodiscard]] constexpr auto operator->() noexcept -> T*
    {
        return &std::get<t_index>(m_variant);
    }

    [[nodiscard]] constexpr auto operator*() const & noexcept -> const T&
    {
        return std::get<t_index>(m_variant);
    }

    [[nodiscard]] constexpr auto operator*() & noexcept -> T&
    {
        return std::get<t_index>(m_variant);
    }

    [[nodiscard]] constexpr auto operator*() const && noexcept -> const T&&
    {
        return std::get<t_index>(std::move(m_variant));
    }

    [[nodiscard]] constexpr auto operator*() && noexcept -> T&&
    {
        return std::get<t_index>(std::move(m_variant));
    }
};


template <class T, class E>
    requires std::is_void_v<T>
class Expected<T, E>
{
public:
    using value_type = T;
    using error_type = E;
    using unexpected_type = Unexpected<E>;

    template <class U>
    using rebind = Expected<U, error_type>;

private:
    template <class U, class G>
    friend class Expected;

    using variant_type = std::variant<std::monostate, std::monostate, E>;

    static constexpr std::size_t t_index = 1;
    static constexpr std::size_t e_index = 2;

    // Declared above, for compatibility with compilers not following CWG 727
    //
    // template <class U>
    // static constexpr bool is_unexpected = false;
    //
    // template <class U>
    // static constexpr bool is_unexpected<Unexpected<U>> = true;

    variant_type m_variant;

public:
    constexpr Expected() : m_variant(std::in_place_index<t_index>)
    {
    }

    Expected(const Expected& other) = default;

    constexpr Expected(const Expected& other)
        requires std::copy_constructible<variant_type> && (!std::is_trivially_copy_constructible_v<variant_type>)
        : m_variant(other.m_variant)
    {
    }

    constexpr Expected(Expected&& other)
        noexcept(std::is_nothrow_move_constructible_v<variant_type>)
        requires std::move_constructible<variant_type> && (!std::is_trivially_move_constructible_v<variant_type>)\
        : m_variant(std::move(other.m_variant))
    {
    }

    template <class U, class G>
    constexpr
    explicit(!std::is_convertible_v<const G&, E>)
    Expected(const Expected<U, G>& other)
        requires std::constructible_from<E, const G&>
        && std::is_void_v<U>
        && (!std::constructible_from<unexpected_type, Expected<U, G>&>)
        && (!std::constructible_from<unexpected_type, Expected<U, G>>)
        && (!std::constructible_from<unexpected_type, const Expected<U, G>&>)
        && (!std::constructible_from<unexpected_type, const Expected<U, G>>)
        : m_variant()
    {
        if (other.has_value())
            m_variant.template emplace<t_index>(std::forward<const U&>(other.value()));
        else
            m_variant.template emplace<e_index>(std::forward<const G&>(other.error()));
    }

    template <class U, class G>
    constexpr
    explicit(!std::is_convertible_v<G, E>)
    Expected(Expected<U, G>&& other)
        requires std::constructible_from<E, G>
        && std::is_void_v<U>
        && (!std::constructible_from<unexpected_type, Expected<U, G>&>)
        && (!std::constructible_from<unexpected_type, Expected<U, G>>)
        && (!std::constructible_from<unexpected_type, const Expected<U, G>&>)
        && (!std::constructible_from<unexpected_type, const Expected<U, G>>)
        : m_variant()
    {
        if (other.has_value())
        {
            m_variant.template emplace<t_index>(std::forward<U>(other.value()));
        }
        else
            m_variant.template emplace<e_index>(std::forward<G>(other.error()));
    }

    template <class G>
    constexpr
    explicit(!std::is_convertible_v<const G&, E>)
    Expected(const Unexpected<G>& e)
        requires std::constructible_from<E, const G&>
        : m_variant(std::in_place_index<e_index>, std::forward<const G&>(e.error()))
    {
    }

    template <class G>
    constexpr explicit(!std::is_convertible_v<G, E>)
    Expected(Unexpected<G>&& e)
        requires std::constructible_from<E, G>
        : m_variant(std::in_place_index<e_index>, std::forward<G>(e.error()))
    {
    }

    constexpr explicit Expected(std::in_place_t)
        : m_variant(std::in_place_index<t_index>)
    {
    }

    template <class... Args>
    constexpr
    explicit
    Expected(Unexpect, Args&&... args)
        requires std::constructible_from<E, Args...>
        : m_variant(std::in_place_index<e_index>, std::forward<Args>(args)...)
    {
    }

    template <class U, class... Args>
    constexpr
    explicit
    Expected(Unexpect, std::initializer_list<U> il, Args&&... args)
        requires std::constructible_from<E, std::initializer_list<U>&, Args...>
        : m_variant(std::in_place_index<e_index>, il, std::forward<Args>(args)...)
    {
    }

    constexpr auto operator=(const Expected& other) -> Expected& = default;

    constexpr auto operator=(Expected&& other) -> Expected& = default;

    template <class G>
    constexpr auto operator=(const Unexpected<G>& e) -> Expected& requires std::constructible_from<E, const G&>
        && std::assignable_from<E&, const G&>
        && (std::is_nothrow_constructible_v<E, const G&> || std::is_nothrow_move_constructible_v<E>)
    {
        m_variant.template emplace<e_index>(std::forward<const G&>(e.error()));
        return *this;
    }

    template <class G>
    constexpr auto operator=(Unexpected<G>&& e) -> Expected& requires std::constructible_from<E, G>
        && std::assignable_from<E&, G>
        && (std::is_nothrow_constructible_v<E, G> || std::is_nothrow_move_constructible_v<E>)
    {
        m_variant.template emplace<e_index>(std::forward<G>(e.error()));
        return *this;
    }

    [[nodiscard]] constexpr auto has_value() const noexcept -> bool
    {
        return m_variant.index() == t_index;
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    constexpr auto value() const & -> void
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    constexpr auto value() && -> void
    {
    }

    [[nodiscard]] constexpr auto error() & -> E&
    {
        return std::get<e_index>(m_variant);
    }

    [[nodiscard]] constexpr auto error() const & -> const E&
    {
        return std::get<e_index>(m_variant);
    }

    [[nodiscard]] constexpr auto error() && -> E&&
    {
        return std::get<e_index>(std::move(m_variant));
    }

    [[nodiscard]] constexpr auto error() const && -> const E&&
    {
        return std::get<e_index>(std::move(m_variant));
    }

    constexpr auto operator*() const noexcept -> void
    {
    }
};

#endif

#endif //EXPECTED_HPP
