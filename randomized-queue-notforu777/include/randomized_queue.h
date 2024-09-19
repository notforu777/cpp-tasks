#pragma once

#include <algorithm>
#include <cstddef>
#include <ios>
#include <random>
#include <vector>

template <class T>
class randomized_queue
{
    std::vector<T> core;
    mutable std::mt19937 engine;
    mutable std::uniform_int_distribution<size_t> distribution;

public:
    template <bool is_const>
    class Iterator
    {
        using It = typename std::conditional<is_const, typename std::vector<T>::const_iterator, typename std::vector<T>::iterator>::type;
        It m_it;
        size_t m_current;
        std::vector<size_t> m_permutation;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = typename std::conditional<is_const, const T, T>::type;
        using pointer = value_type *;
        using reference = value_type &;
        using iterator_category = std::forward_iterator_tag;

        Iterator()
            : m_it(m_permutation.begin())
            , m_current(0)
        {
        }

        Iterator(It it, size_t current_pos, size_t size)
            : m_it(it)
            , m_current(current_pos)
            , m_permutation(size)
        {
            for (size_t i = 0; i < size; ++i) {
                m_permutation[i] = i;
            }
            thread_local static std::mt19937 inner_eng(std::random_device{}());
            std::shuffle(m_permutation.begin(), m_permutation.end(), inner_eng);
        }

        ~Iterator() = default;

        reference operator*()
        {
            if (m_permutation.empty()) {
                throw std::out_of_range("incorrect operator");
            }
            size_t addition;
            m_current == m_permutation.size() ? addition = std::numeric_limits<size_t>::max() : addition = m_permutation[m_current];
            return *(m_it + addition);
        }

        pointer operator->()
        {
            if (m_permutation.empty()) {
                throw std::out_of_range("incorrect operator");
            }
            size_t addition;
            m_current == m_permutation.size() ? addition = std::numeric_limits<size_t>::max() : addition = m_permutation[m_current];
            return (m_it + addition).operator->();
        }

        Iterator & operator++()
        {
            if (m_permutation.empty()) {
                throw std::out_of_range("incorrect iterator");
            }
            m_current++;
            return *this;
        }

        Iterator operator++(int)
        {
            if (m_permutation.empty()) {
                throw std::out_of_range("incorrect iterator");
            }
            auto tmp = *this;
            operator++();
            return tmp;
        }

        friend bool operator==(const Iterator & left, const Iterator & right)
        {
            if (left.m_it == right.m_it && left.m_permutation.empty() && right.m_permutation.empty()) {
                return true;
            }
            if (left.m_permutation.empty() || right.m_permutation.empty()) {
                return false;
            }
            size_t left_addition;
            left.m_current == left.m_permutation.size() ? left_addition = std::numeric_limits<size_t>::max() : left_addition = left.m_permutation[left.m_current];
            size_t right_addition;
            right.m_current == right.m_permutation.size() ? right_addition = std::numeric_limits<size_t>::max() : right_addition = right.m_permutation[right.m_current];
            return left.m_it == right.m_it && left_addition == right_addition;
        }

        friend bool operator!=(const Iterator & left, const Iterator & right)
        {
            return !(left == right);
        }
    };

    randomized_queue()
    {
        std::random_device rd;
        engine = std::mt19937(rd());
    }

    bool empty() const
    {
        return core.empty();
    }

    size_t size() const
    {
        return core.size();
    }

    const T & sample() const
    {
        return core[distribution(engine) % core.size()];
    }

    T & sample()
    {
        return core[distribution(engine) % core.size()];
    }

    void enqueue(const T & x)
    {
        core.emplace_back(x);
    }

    void enqueue(T && x)
    {
        core.emplace_back(std::move(x));
    }

    T dequeue()
    {
        if (core.empty()) {
            throw std::ios_base::failure("dequeue from empty queue");
        }
        std::swap(core[distribution(engine) % core.size()], core.back());
        T element = std::move(core.back());
        core.pop_back();
        return element;
    }

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    iterator begin()
    {
        return iterator(core.begin(), 0, core.size());
    }

    const_iterator begin() const
    {
        return const_iterator(core.begin(), 0, core.size());
    }

    iterator end()
    {
        return iterator(core.begin(), core.size(), core.size());
    }

    const_iterator end() const
    {
        return const_iterator(core.begin(), core.size(), core.size());
    }

    const_iterator cbegin() const
    {
        return const_iterator(core.cbegin(), 0, core.size());
    }

    const_iterator cend() const
    {
        return const_iterator(core.cbegin(), core.size(), core.size());
    }
};
