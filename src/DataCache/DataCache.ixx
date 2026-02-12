//
// Created by scros on 2/7/26.
//

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

auto writeFile(const std::filesystem::path & path,
               const std::span<const std::byte> data) -> std::expected<void, std::string>
{
    if (const auto parent = path.parent_path(); !parent.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec)
        {
            return std::unexpected<std::string>(std::in_place, mapError(ec));
        }
    }

    std::ofstream file(path, std::ios::binary);
    if (!file)
    {
        return std::unexpected<std::string>(std::in_place, "PermissionDenied");
    }

    file.write(reinterpret_cast<const char *>(data.data()), data.size());
    if (file.fail())
    {
        std::unexpected<std::string>(std::in_place, "ReadError");
    }

    return {};
}

auto readFile(const std::filesystem::path & path) -> std::optional<std::expected<std::vector<std::byte>, std::string> >
{
    std::error_code ec;
    if (!std::filesystem::exists(path, ec))
    {
        if (ec)
        {
            return std::unexpected<std::string>(std::in_place, mapError(ec));
        }
        return std::nullopt;
    }

    const auto size = std::filesystem::file_size(path, ec);
    if (ec)
    {
        return std::unexpected<std::string>(std::in_place, mapError(ec));
    }

    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        return std::unexpected<std::string>(std::in_place, "PermissionDenied");
    }

    std::vector<std::byte> buffer(size);
    file.read(reinterpret_cast<char *>(buffer.data()), size);

    if (file.gcount() != static_cast<std::streamsize>(size))
    {
        return std::unexpected<std::string>(std::in_place, "ReadError");
    }

    return buffer;
}

export template<class T>
concept CacheDataSerializable = requires(const T & t, const std::vector<std::byte> & data)
{
    { T::serialize(t) } -> std::same_as<std::vector<std::byte>>;
    { T::deserialize(data) } -> std::same_as<std::expected<T, std::string> >;
};

export class DataCache
{
public:
    template<CacheDataSerializable T>
    static auto load(const std::filesystem::path & path) -> std::optional<std::expected<T, std::string> >
    {
        return readFile(path).transform(andThenWith(T::deserialize));
    }

    template<CacheDataSerializable T>
    static auto save(const std::filesystem::path & path, const T & data) -> std::expected<void, std::string>
    {
        return writeFile(path, T::serialize(data));
    }

    template<CacheDataSerializable T, class F>
    requires std::invocable<F> && std::same_as<std::invoke_result_t<F>, std::expected<T, std::string>>
    static auto loadOrCreate(const std::filesystem::path & path, F&& generator) -> std::expected<T, std::string>
    {
        return load<T>(path).or_else([&path, generator] -> std::optional<std::expected<T, std::string>>
        {
            TRY_V(auto, data, generator());
            TRY(save<T>(path, data));
            return std::expected<T, std::string>(std::move(data));
        }).value();
    }
};
