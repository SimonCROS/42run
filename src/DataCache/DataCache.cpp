//
// Created by scros on 2/22/26.
//

module DataCache;
auto DataCache::writeFile(const std::filesystem::path & path,
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

auto DataCache::readFile(
    const std::filesystem::path & path) -> std::optional<std::expected<std::vector<std::byte>, std::string> >
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
