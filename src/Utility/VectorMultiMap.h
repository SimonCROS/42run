//
// Created by Simon Cros on 08/02/2025.
//

#ifndef VECTORMULTIMAP_H
#define VECTORMULTIMAP_H

#include <algorithm>
#include <vector>

template <typename Key, typename Value>
class VectorMultiMap
{
public:
    struct Entry
    {
        Key key;
        Value value;

        bool operator<(const Entry& other) const
        {
            return key < other.key;
        }

        bool operator<(const Key& other) const
        {
            return key < other;
        }
    };

    using Container = std::vector<Entry>;
    using Iterator = typename Container::iterator;
    using ConstIterator = typename Container::const_iterator;

    auto insert(const Key& key, const Value& value) -> Iterator
    {
        Entry entry{key, value};
        auto it = std::lower_bound(m_data.begin(), m_data.end(), entry);
        return m_data.insert(it, entry);
    }

    template <class... Args>
        requires std::constructible_from<Value, Args...>
    auto emplace(const Key& key, Args&&... args) -> Iterator
    {
        auto it = std::lower_bound(m_data.begin(), m_data.end(), key);
        return m_data.emplace(it, key, std::forward<Args>(args)...);
    }

    [[nodiscard]] auto equal_range(const Key& key) -> std::pair<Iterator, Iterator>
    {
        return std::equal_range(m_data.begin(), m_data.end(), key);
    }

    [[nodiscard]] auto equal_range(const Key& key) const -> std::pair<ConstIterator, ConstIterator>
    {
        return std::equal_range(m_data.begin(), m_data.end(), key);
    }

    auto erase(const Key& key) -> void
    {
        auto range = equal_range(key);
        m_data.erase(range.first, range.second);
    }

    auto erase(const ConstIterator& it) -> void
    {
        m_data.erase(it);
    }

    [[nodiscard]] auto size() -> typename Container::size_type { return m_data.size(); }

    [[nodiscard]] auto begin() -> Iterator { return m_data.begin(); }
    [[nodiscard]] auto end() -> Iterator { return m_data.end(); }
    [[nodiscard]] auto begin() const -> ConstIterator { return m_data.begin(); }
    [[nodiscard]] auto end() const -> ConstIterator { return m_data.end(); }

private:
    Container m_data{};
};

#endif //VECTORMULTIMAP_H
