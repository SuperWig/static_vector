// Copyright (c) Daniel Marshall.
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <algorithm>
#include <cassert>
#include <compare>
#include <memory>
#include <type_traits>
#include <utility>

namespace dpm
{
    namespace
    {
        namespace ranges = std::ranges;
    }

    template <class T, std::size_t N>
    struct uninitialized_storage
    {
        alignas(T) std::byte storage[sizeof(T) * N];

        T* data() { return reinterpret_cast<T*>(std::addressof(storage)); }
        const T* data() const { return reinterpret_cast<const T*>(std::addressof(storage)); }
    };

    template <class T, std::size_t Capacity>
    class static_vector
    {
        static_assert(!std::is_const_v<T>, "static_vector can't contain const elements.");

        uninitialized_storage<T, Capacity> storage_;
        std::size_t size_ = 0;

        constexpr static bool trivial_copy_ctor = std::is_trivially_copy_constructible_v<T>;
        constexpr static bool trivial_move_ctor = std::is_trivially_move_constructible_v<T>;
        constexpr static bool trivial_copy_assignable = std::is_trivially_copy_assignable_v<T>;
        constexpr static bool trivial_move_assignable = std::is_trivially_move_assignable_v<T>;
        constexpr static bool trivial_dtor = std::is_trivially_destructible_v<T>;

    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = value_type&;
        using const_reference = const value_type&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using iterator = T*;
        using const_iterator = const T*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // 5.2, trivial copy/move construction:
        static_vector() = default;
        static_vector(const static_vector& other) requires trivial_copy_ctor = default;
        static_vector(static_vector&& other) requires trivial_move_ctor = default;

        // 5.2, non-trivial copy/move construction:
        constexpr static_vector(const static_vector& other) noexcept(std::is_nothrow_constructible_v<value_type>)
            : size_(other.size_)
        {
            static_assert(std::is_copy_constructible_v<value_type>, "value_type must be copy constructible.");
            ranges::uninitialized_copy(other, *this);
        }
        constexpr static_vector(static_vector&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>)
            : size_(std::exchange(other.size_, 0))
        {
            static_assert(std::is_move_constructible_v<value_type>, "value_type must be move constructible.");
            ranges::uninitialized_move_n(other.begin(), size_, begin(), end());
        }
        constexpr explicit static_vector(size_type count) : size_(count)
        {
            static_assert(std::is_default_constructible_v<value_type>, "value_type must be default constructible.");
            assert(count <= capacity());
            ranges::uninitialized_default_construct(*this);
        }
        constexpr explicit static_vector(size_type count, const value_type& value) : size_(count)
        {
            static_assert(std::is_copy_constructible_v<value_type>, "value_type must be copy constructible.");
            assert(count <= capacity());
            ranges::uninitialized_fill(*this, value);
        }
        template <std::input_iterator InputIter>
        constexpr static_vector(InputIter first, InputIter last)
        {
            static_assert(std::is_constructible_v<value_type, decltype(*first)>,
                "value_type must be constructible from decltype(*first)");
            size_ = std::distance(first, last);
            assert(size_ <= capacity());
            ranges::uninitialized_copy(first, last, begin(), end());
        }
        constexpr static_vector(std::initializer_list<value_type> il) : size_(il.size())
        {
            assert(il.size() <= capacity());
            ranges::uninitialized_copy(il, *this);
        }

        // 5.3, copy/move assignment:
        static_vector& operator=(const static_vector& other) requires trivial_copy_assignable = default;
        static_vector& operator=(static_vector&& other) requires trivial_move_assignable = default;

        constexpr static_vector& operator=(const static_vector& other) noexcept(
            std::is_nothrow_copy_assignable_v<value_type>)
        {
            assign(other.begin(), other.end());
            return *this;
        }
        constexpr static_vector& operator=(static_vector&& other) noexcept(
            std::is_nothrow_move_assignable_v<value_type>)
        {
            auto amount = std::min(size_, other.size_);
            auto [move_end, this_begin] = ranges::copy_n(std::make_move_iterator(other.begin()), amount, begin());
            ranges::destroy(this_begin, end());

            size_ = other.size_;
            ranges::uninitialized_move(move_end, std::make_move_iterator(other.end()), this_begin, end());
            ranges::destroy(other);
            other.size_ = 0;
            return *this;
        }
        template <std::input_iterator InputIterator>
        constexpr void assign(InputIterator first, InputIterator last)
        {
            auto new_size = std::distance(first, last);
            auto min = std::min<ptrdiff_t>(size_, new_size);

            auto [in, out] = ranges::copy_n(first, min, begin());
            auto [_, new_end] = ranges::uninitialized_copy(in, last, out, end());
            ranges::destroy(new_end, end());
            size_ = new_size;
        }
        constexpr void assign(size_type n, const value_type& value)
        {
            auto min = std::min(size_, n);
            auto fill_end = ranges::fill_n(begin(), min, value);
            auto new_end = ranges::uninitialized_fill_n(fill_end, n - min, value);
            size_ = n;
            ranges::destroy(new_end, end());
        }
        constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }

        // 5.4, destruction
        constexpr ~static_vector() noexcept requires trivial_dtor = default;
        constexpr ~static_vector() { ranges::destroy_n(begin(), size_); }

        // iterators
        [[nodiscard]] constexpr iterator begin() noexcept { return data(); }
        [[nodiscard]] constexpr const_iterator begin() const noexcept { return data(); }
        [[nodiscard]] constexpr iterator end() noexcept { return data() + size_; }
        [[nodiscard]] constexpr const_iterator end() const noexcept { return data() + size_; }
        [[nodiscard]] constexpr reverse_iterator rbegin() noexcept { std::make_reverse_iterator(end()); }
        [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { std::make_reverse_iterator(end()); }
        [[nodiscard]] constexpr reverse_iterator rend() noexcept { std::make_reverse_iterator(begin()); }
        [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { std::make_reverse_iterator(begin()); }
        [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
        [[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }
        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return rend(); }

        // 5.5, size/capacity:
        [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
        [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
        [[nodiscard]] static constexpr size_type max_size() noexcept { return Capacity; }
        [[nodiscard]] static constexpr size_type capacity() noexcept { return Capacity; }

        constexpr void resize(size_type sz)
        {
            static_assert(std::is_default_constructible_v<value_type>, "T must be default constuctible");
            assert(sz <= capacity());
            if (sz < size_)
            {
                const auto amount = size_ - sz;
                ranges::destroy_n(data() + sz, amount);
            }
            else
            {
                const auto amount = sz - size_;
                ranges::uninitialized_default_construct_n(end(), amount);
            }
            size_ = sz;
        }
        constexpr void resize(size_type sz, const value_type& value)
        {
            assert(sz <= capacity());
            if (sz < size_)
            {
                const auto amount = size_ - sz;
                ranges::destroy_n(data() + sz, amount);
            }
            else
            {
                const auto amount = sz - size_;
                ranges::uninitialized_fill_n(end(), amount, value);
            }
            size_ = sz;
        }

        // 5.6, element and data access:
        [[nodiscard]] constexpr reference operator[](size_t n) noexcept
        {
            assert(n < size_ && n >= 0);
            return data()[n];
        }
        [[nodiscard]] constexpr const_reference operator[](size_t n) const noexcept
        {
            assert(n < size_ && n >= 0);
            return data()[n];
        }

        [[nodiscard]] constexpr reference front() { return *begin(); }
        [[nodiscard]] constexpr const_reference front() const { return *begin(); }

        [[nodiscard]] constexpr reference back() { return *(begin() + size_ - 1); }
        [[nodiscard]] constexpr const_reference back() const { return *(begin() + size_ - 1); }

        [[nodiscard]] constexpr pointer data() noexcept { return std::launder(storage_.data()); }
        [[nodiscard]] constexpr const_pointer data() const noexcept { return std::launder(storage_.data()); }

        // 5.7, modifiers:
        constexpr iterator insert(const_iterator position, const value_type& x)
        {
            assert(size_ < capacity());
            emplace_back(x);
            return ranges::rotate(const_cast<iterator>(position), end() - 1, end()).begin() - 1;
        }
        constexpr iterator insert(const_iterator position, value_type&& x)
        {
            assert(size_ < capacity());
            emplace_back(std::move(x));
            return ranges::rotate(const_cast<iterator>(position), end() - 1, end()).begin() - 1;
        }
        constexpr iterator insert(const_iterator position, size_type n, const value_type& x)
        {
            assert(size_ + n <= capacity());
            auto old_end = end();
            for (size_t i = 0; i < n; ++i)
            {
                emplace_back(x);
            }
            auto pos = const_cast<iterator>(position);
            ranges::rotate(pos, old_end, end());
            return pos;
        }
        template <std::input_iterator InputIterator>
        constexpr iterator insert(const_iterator position, InputIterator first, InputIterator last)
        {
            auto old_end = end();
            size_ += std::distance(first, last);
            assert(size_ <= capacity());
            ranges::uninitialized_copy(first, last, old_end, end());

            auto pos = const_cast<iterator>(position);
            ranges::rotate(pos, old_end, end());
            return pos;
        }
        constexpr iterator insert(const_iterator position, std::initializer_list<value_type> il)
        {
            return insert(position, il.begin(), il.end());
        }

        template <class... Args>
        constexpr iterator emplace(const_iterator position, Args&&... args)
        {
            assert(size_ < capacity());
            emplace_back(std::forward<Args>(args)...);
            return ranges::rotate(iterator(position), end() - 1, end()).begin() - 1;
        }

        template <class... Args>
        constexpr reference emplace_back(Args&&... args)
        {
            assert(size_ < capacity());
            auto* emplaced = std::construct_at(end(), std::forward<Args>(args)...);
            ++size_;
            return *emplaced;
        }
        constexpr void push_back(const value_type& x) { emplace_back(x); }
        constexpr void push_back(value_type&& x) { emplace_back(std::move(x)); }

        constexpr void pop_back()
        {
            std::destroy_at(std::addressof(back()));
            --size_;
        }
        constexpr iterator erase(const_iterator position)
        {
            std::destroy_at(position);
            auto pos = const_cast<iterator>(position);
            ranges::rotate(pos, pos + 1, end());
            --size_;
            return pos;
        }
        constexpr iterator erase(const_iterator first, const_iterator last)
        {
            auto removed_end = const_cast<iterator>(ranges::destroy(first, last));
            ranges::rotate(const_cast<iterator>(first), removed_end, end());
            size_ -= std::distance(first, last);
            return removed_end;
        }

        constexpr void clear() noexcept
        {
            ranges::destroy(*this);
            size_ = 0;
        }

        constexpr void swap(static_vector& other) noexcept(
            std::is_nothrow_swappable_v<value_type>&& std::is_nothrow_move_constructible_v<value_type>) requires
            std::is_move_constructible_v<value_type> && std::is_swappable_v<value_type>
        {
            // Could this be simpler?
            auto [smaller_begin, smaller_end, larger_begin, larger_end] = [&] {
                if (size_ < other.size_)
                {
                    return std::array<iterator, 4>{ begin(), end(), other.begin(), other.end() };
                }
                return std::array<iterator, 4>{ other.begin(), other.end(), begin(), end() };
            }();

            for (; smaller_begin != smaller_end; ++smaller_begin, ++larger_begin)
            {
                ranges::iter_swap(smaller_begin, larger_begin);
            }
            ranges::uninitialized_move(larger_begin, larger_end, smaller_begin, std::unreachable_sentinel);
            ranges::destroy(larger_begin, larger_end);
            std::swap(size_, other.size_);
        }

        [[nodiscard]] constexpr bool operator==(const static_vector& other) const noexcept
        {
            return ranges::equal(*this, other);
        }
        [[nodiscard]] constexpr auto operator<=>(const static_vector& other) const noexcept
        {
            return std::lexicographical_compare_three_way(begin(), end(), other.begin(), other.end());
        }
    };

    // 5.8, specialized algorithms:
    template <typename T, size_t N>
    constexpr void swap(static_vector<T, N>& x, static_vector<T, N>& y) noexcept(noexcept(x.swap(y)))
    {
        x.swap(y);
    }

}
