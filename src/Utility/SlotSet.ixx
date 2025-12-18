//
// Created by Simon Cros on 2/19/25.
//

export module Utility.SlotSet;
import std.compat;

export struct SlotSetIndex
{
    int32_t value = -1;

    static constexpr auto invalid() noexcept -> SlotSetIndex { return {}; }

    constexpr SlotSetIndex() noexcept = default;

    constexpr SlotSetIndex(const SlotSetIndex &) noexcept = default;

    constexpr SlotSetIndex(SlotSetIndex &&) noexcept = default;

    constexpr SlotSetIndex & operator=(const SlotSetIndex &) noexcept = default;

    constexpr SlotSetIndex & operator=(SlotSetIndex &&) noexcept = default;

    constexpr explicit SlotSetIndex(const int32_t index) noexcept : value(index) {}

    constexpr auto operator==(const SlotSetIndex & other) const noexcept -> bool = default;

    constexpr auto operator<=>(const SlotSetIndex & other) const noexcept -> std::strong_ordering = default;

    [[nodiscard]] constexpr auto isValid() const noexcept -> bool { return *this != invalid(); }
};

struct SlotSetValueIndex
{
    int32_t value = -1;

    static constexpr auto invalid() -> SlotSetValueIndex { return {}; }

    constexpr SlotSetValueIndex() = default;

    constexpr SlotSetValueIndex(const SlotSetValueIndex &) = default;

    constexpr SlotSetValueIndex(SlotSetValueIndex &&) = default;

    constexpr SlotSetValueIndex & operator=(const SlotSetValueIndex &) = default;

    constexpr SlotSetValueIndex & operator=(SlotSetValueIndex &&) = default;

    constexpr explicit SlotSetValueIndex(const int32_t index) : value(index) {}

    constexpr auto operator==(const SlotSetValueIndex & other) const -> bool = default;

    constexpr auto operator<=>(const SlotSetValueIndex & other) const -> std::strong_ordering = default;
};

template<class T>
concept Indexed = requires(T a)
{
    requires std::same_as<decltype(a.index), SlotSetIndex>;
    requires std::swappable<T>;
    a.index = std::declval<SlotSetIndex>();
};

export
template<Indexed T>
class SlotSet
{
public:
    using Value = T;

    using ValueContainer = std::deque<Value>;
    using Iterator = typename ValueContainer::iterator;
    using ConstIterator = typename ValueContainer::const_iterator;

    using SlotContainer = std::deque<SlotSetValueIndex>;
    using FreeSlotContainer = std::priority_queue<SlotSetIndex>;

private:
    ValueContainer m_values; // tightly-packed values
    SlotContainer m_slots; // slots with holes
    FreeSlotContainer m_freeSlots; // indices of holes in m_slots

public:
    SlotSet() = default;

    template<class... Args>
        requires std::constructible_from<Value, Args...> || std::move_constructible<T>
    auto emplace(Args &&... args) -> Value &
    {
        const SlotSetValueIndex valueIndex = SlotSetValueIndex(m_values.size());
        SlotSetIndex slotIndex;
        if (m_freeSlots.empty())
        {
            slotIndex = SlotSetIndex(m_slots.size());
            m_slots.emplace_back(valueIndex);
        }
        else
        {
            slotIndex = m_freeSlots.top();
            m_freeSlots.pop();
            m_slots[slotIndex.value] = valueIndex;
        }

        auto & ref = m_values.emplace_back(std::forward<Args>(args)...);
        ref.index = slotIndex;
        return ref;
    }

    auto erase(const SlotSetIndex index) -> bool
    {
        if (index == SlotSetIndex::invalid())
        {
            return false;
        }

        const SlotSetValueIndex valueIndex = m_slots[index.value];
        const SlotSetValueIndex lastValueIndex = SlotSetValueIndex(m_values.size() - 1);

        m_freeSlots.emplace(index.value);
        m_slots[index.value] = SlotSetValueIndex::invalid();
        if (valueIndex != lastValueIndex)
        {
            const auto & last = m_values[lastValueIndex];
            m_slots[last.index] = valueIndex;
            std::swap(m_values[valueIndex], last);
        }
        m_values.pop_back();
        return true;
    }

    [[nodiscard]] auto size() -> typename ValueContainer::size_type { return m_values.size(); }

    [[nodiscard]] auto begin() -> Iterator { return m_values.begin(); }
    [[nodiscard]] auto end() -> Iterator { return m_values.end(); }
    [[nodiscard]] auto begin() const -> ConstIterator { return m_values.begin(); }
    [[nodiscard]] auto end() const -> ConstIterator { return m_values.end(); }

    [[nodiscard]] auto operator[](const SlotSetIndex index) -> Value &
    {
        return m_values[m_slots[index.value].value];
    }

    [[nodiscard]] auto operator[](const SlotSetIndex index) const -> const Value &
    {
        return m_values[m_slots[index.value].value];
    }
};
