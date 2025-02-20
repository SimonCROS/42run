//
// Created by Simon Cros on 2/19/25.
//

#ifndef SLOTSET_H
#define SLOTSET_H
#include <deque>
#include <cstdint>
#include <queue>
#include <concepts>

using SlotSetIndex = int32_t;

// template <class T>
// concept Indexed = requires(T a)
// {
//     { a.index } -> std::same_as<SlotSetIndex>;
//     a.index = std::declval<SlotSetIndex>();
// };

template <class T>
class SlotSet
{
public:
    using Value = T;
    using Index = SlotSetIndex;

    using ValueContainer = std::deque<Value>;
    using Iterator = typename ValueContainer::iterator;
    using ConstIterator = typename ValueContainer::const_iterator;

    using SlotContainer = std::deque<Index>;
    using FreeSlotContainer = std::queue<Index>;

private:
    ValueContainer m_values; // tightly-packed values
    SlotContainer m_slots; // slots with holes
    FreeSlotContainer m_freeSlots; // indices of holes in m_slots

public:
    SlotSet() = default;

    template <class... Args>
        requires std::constructible_from<Value, Args...>
    auto emplace(Args&&... args) -> Value&
    {
        Index valueIndex = m_values.size();
        Index index;
        if (m_freeSlots.empty())
        {
            index = m_slots.size();
            m_slots.emplace_back(valueIndex);
        }
        else
        {
            index = m_freeSlots.front();
            m_freeSlots.pop();
            m_slots[index] = valueIndex;
        }

        auto& ref = m_values.emplace_back(std::forward<Args>(args)...);
        ref.index = index;
        return ref;
    }

    auto erase(const Index index) -> bool
    {
        Index valueIndex = m_slots[index];
        Index lastValueIndex = m_values.size() - 1;

        m_freeSlots.emplace(index);
        m_slots[index] = 0;
        if (valueIndex != lastValueIndex)
        {
            auto& other = m_values[lastValueIndex];
            m_slots[other.index] = valueIndex;
            other.index = valueIndex;

            std::swap(m_values[valueIndex], other);
        }
        m_values.pop_back();
        return true; // currently always true
    }

    [[nodiscard]] auto size() -> typename ValueContainer::size_type { return m_values.size(); }

    [[nodiscard]] auto begin() -> Iterator { return m_values.begin(); }
    [[nodiscard]] auto end() -> Iterator { return m_values.end(); }
    [[nodiscard]] auto begin() const -> ConstIterator { return m_values.begin(); }
    [[nodiscard]] auto end() const -> ConstIterator { return m_values.end(); }

    [[nodiscard]] auto operator[](const Index index) -> Value& { return m_values[m_slots[index]]; }

    // Object:
    //   int index
    //   int version
    //   other data
    //
    // SlotMap:
    //   Object objects[]
    //   int slots[]
    //   int freelist[]
    //   int count
    //
    //   Get(id):
    //     index = indirection[id.index]
    //     if objects[index].version = id.version:
    //       return &objects[index]
    //     else:
    //       return null
    //
    //   CreateObject():
    //     index = freelist.pop()
    //
    //     objects[count].index = id
    //     objects[count].version += 1
    //
    //     indirection[index] = count
    //
    //     Object* object = &objects[count].object
    //     object.initialize()
    //
    //     count += 1
    //
    //     return object
    //
    //   Remove(id):
    //     index = indirection[id.index]
    //     if objects[index].version = id.version:
    //       objects[index].version += 1
    //       objects[count - 1].version += 1
    //
    //       swap(objects[index].data, objects[count - 1].data)
};

#endif //SLOTSET_H
