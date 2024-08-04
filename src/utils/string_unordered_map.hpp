#ifndef STRING_UNORDERED_MAP
#define STRING_UNORDERED_MAP

#include <string>
#include <string_view>
#include <unordered_map>

struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;

    std::size_t operator()(const char* str) const { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const { return hash_type{}(str); }
    std::size_t operator()(const std::string& str) const { return hash_type{}(str); }
};

template <
    typename Value,
    typename Allocator = std::allocator<std::pair<std::string, Value>>>
using string_unordered_map = std::unordered_map<std::string, Value, string_hash, std::equal_to<>, Allocator>;

#endif
