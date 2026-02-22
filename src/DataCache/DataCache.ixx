//
// Created by scros on 2/7/26.
//

module;
#include <cstdio>

export module DataCache;
import std;

constexpr auto andThenWith(auto func)
{
    return [func]<typename T0>(T0 && monad) { return std::forward<T0>(monad).and_then(func); };
}

auto mapError(const std::error_code & ec) -> std::string_view
{
    if (ec == std::errc::permission_denied) return "PermissionDenied";
    if (ec == std::errc::no_space_on_device) return "DiskFull";
    if (ec == std::errc::io_error) return "ReadError";
    return "Unknown";
}

export template<class T>
concept CacheDataSerializable = requires(const T & t, const std::vector<std::byte> & data)
{
    { T::serialize(t) } -> std::same_as<std::vector<std::byte> >;
    { T::deserialize(data) } -> std::same_as<std::expected<T, std::string> >;
};

export class DataCache
{
public:
    [[nodiscard]]
    static auto writeFile(const std::filesystem::path & path,
                          std::span<const std::byte> data) -> std::expected<void, std::string>;

    [[nodiscard]]
    static auto readFile(
        const std::filesystem::path & path) -> std::optional<std::expected<std::vector<std::byte>, std::string> >;

    // template<CacheDataSerializable T>
    // static auto load(const std::filesystem::path & path) -> std::optional<std::expected<T, std::string> >
    // {
    //     return readFile(path).transform(andThenWith(T::deserialize));
    // }
    //
    // template<CacheDataSerializable T>
    // static auto save(const std::filesystem::path & path, const T & data) -> std::expected<void, std::string>
    // {
    //     return writeFile(path, T::serialize(data));
    // }
    //
    // template<CacheDataSerializable T, class F>
    // requires std::invocable<F> && std::same_as<std::invoke_result_t<F>, std::expected<T, std::string>>
    // static auto loadOrCreate(const std::filesystem::path & path, F&& generator) -> std::expected<T, std::string>
    // {
    //     return load<T>(path)
    //     .and_then([&path](std::expected<T, std::string> result) -> std::optional<std::expected<T, std::string>>
    //     {
    //         if (result)
    //         {
    //             return result;
    //         }
    //
    //         std::println(stderr, "Failed to deserialize {}: {}", path.c_str(), result.error());
    //         return std::nullopt;
    //     }).or_else([&path, generator] -> std::optional<std::expected<T, std::string>>
    //     {
    //         TRY_V(auto, data, generator());
    //         TRY(save<T>(path, data));
    //         return std::expected<T, std::string>(std::move(data));
    //     }).value();
    // }
};
