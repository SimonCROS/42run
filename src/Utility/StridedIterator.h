//
// Created by Simon Cros on 01/02/2025.
//

#ifndef STRIDEDITERATOR_H
#define STRIDEDITERATOR_H
#include <iterator>

template <class Iterator>
class StridedIterator
{
public:
    using difference_type = typename std::iterator_traits<Iterator>::difference_type;
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    using pointer = typename std::iterator_traits<Iterator>::pointer;
    using reference = typename std::iterator_traits<Iterator>::reference;
    using iterator_category = std::random_access_iterator_tag;

private:
    Iterator m_it;
    difference_type m_stride;

public:
    StridedIterator(Iterator it, difference_type stride) : m_it(it), m_stride(stride)
    {
    }

    auto operator*() const -> reference { return *m_it; }

    auto operator++() -> StridedIterator&
    {
        m_it += m_stride;
        return *this;
    }

    auto operator++(int) -> StridedIterator
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    auto operator--() -> StridedIterator&
    {
        m_it -= m_stride;
        return *this;
    }

    auto operator--(int) -> StridedIterator
    {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    auto operator+=(difference_type n) -> StridedIterator&
    {
        m_it += n * m_stride;
        return *this;
    }

    auto operator+(difference_type n) const -> StridedIterator
    {
        auto tmp = *this;
        return tmp += n;
    }

    auto operator-=(difference_type n) -> StridedIterator&
    {
        m_it -= n * m_stride;
        return *this;
    }

    auto operator-(difference_type n) const -> StridedIterator
    {
        auto tmp = *this;
        return tmp -= n;
    }

    auto operator-(const StridedIterator& other) const -> difference_type { return (m_it - other.m_it) / m_stride; }
    auto operator==(const StridedIterator& other) const -> bool { return m_it == other.m_it; }
    auto operator!=(const StridedIterator& other) const -> bool { return !(*this == other); }
    auto operator<(const StridedIterator& other) const -> bool { return m_it < other.m_it; }
};

#endif //STRIDEDITERATOR_H
