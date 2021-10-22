// Copyright (c) Daniel Marshall.
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>

namespace dpm
{
    template <class T, std::size_t N>
    struct uninitialized_storage
    {
        alignas(T) std::byte storage[sizeof(T) * N];

        T* data()
        {
            return reinterpret_cast<T*>(&storage);
        }
        const T* data() const
        {
            return reinterpret_cast<const T*>(&storage);
        }
    };
    template <class T, std::size_t Capacity>
    class static_vector
    {
        using storage_type = std::conditional_t<std::is_trivial_v<T>, std::array<T, Capacity>, uninitialized_storage<T, Capacity>>;
        storage_type storage_;
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
        constexpr static_vector(const static_vector& other) : size_(other.size_)
        {
            std::ranges::uninitialized_copy(other, *this);
        }
        constexpr static_vector(static_vector&& other) noexcept : size_(std::exchange(other.size_, 0))
        {
            static_assert(std::is_move_constructible_v<value_type>, "value_type must be move constructible.");
            std::ranges::uninitialized_move_n(other.begin(), size_, begin(), end());
        }
        constexpr explicit static_vector(size_type count) : size_(count)
        {
            static_assert(std::is_default_constructible_v<value_type>, "value_type must be default constructible.");
            assert(count <= capacity());
            std::ranges::uninitialized_default_construct(*this);
        }
        constexpr explicit static_vector(size_type count, const value_type& value) : size_(count)
        {
            static_assert(std::is_copy_constructible_v<value_type>, "value_type must be copy constructible.");
            assert(count <= capacity());
            std::ranges::uninitialized_fill(*this, value);
        }
        template <std::input_iterator InputIter>
        constexpr static_vector(InputIter first, InputIter last)
        {
            static_assert(std::is_constructible_v<value_type, decltype(*first)>, "value_type must be constructible from decltype(*first)");
            size_ = std::distance(first, last);
            assert(size_ <= capacity());
            std::ranges::uninitialized_copy(first, last, begin(), end());
        }
        constexpr static_vector(std::initializer_list<value_type> il) : size_(il.size())
        {
            std::ranges::uninitialized_copy(il, *this);
        }

        // 5.3, copy/move assignment:
        static_vector& operator=(const static_vector& other) requires trivial_copy_assignable = default;
        static_vector& operator=(static_vector&& other) requires trivial_move_assignable = default;

        constexpr static_vector& operator=(const static_vector& other) noexcept(std::is_nothrow_copy_assignable_v<value_type>)
        {
            if (size_ >= other.size_)
            {
                const auto [_, out] = std::ranges::copy(other, begin());
                std::ranges::destroy(out, end());
                size_ = other.size_;
            }
            else
            {
                auto [copy_end, this_begin] = std::ranges::copy_n(other.begin(), size_, begin());
                size_ = other.size_;
                std::ranges::uninitialized_copy(copy_end, other.end(), this_begin, end());
            }
            return *this;
        }
        constexpr static_vector& operator=(static_vector&& other) noexcept(std::is_nothrow_move_assignable_v<value_type>)
        {
            if (size_ >= other.size_)
            {
                const auto [_, out] = std::ranges::move(other, begin());
                std::ranges::destroy(out, end());
                size_ = other.size_;
            }
            else
            {
                auto [move_end, this_begin] = std::ranges::copy_n(std::make_move_iterator(other.begin()), size_, begin());
                size_ = other.size_;
                std::ranges::uninitialized_move(move_end, std::make_move_iterator(other.end()), this_begin, end());
            }
            other.size_ = 0;
            return *this;
        }
        //      template <class InputIterator>
        // TODO: constexpr void assign(InputIterator first, InputIterator last);
        // TODO: constexpr void assign(size_type n, const value_type& u);
        // TODO: constexpr void assign(std::initializer_list<value_type> il);

        // 5.4, destruction
        constexpr ~static_vector() noexcept requires trivial_dtor = default;
        constexpr ~static_vector()
        {
            std::ranges::destroy_n(begin(), size_);
        }

        // iterators
        [[nodiscard]] constexpr iterator begin() noexcept
        {
            return data();
        }
        [[nodiscard]] constexpr const_iterator begin() const noexcept
        {
            return data();
        }
        [[nodiscard]] constexpr iterator end() noexcept
        {
            return data() + size_;
        }
        [[nodiscard]] constexpr const_iterator end() const noexcept
        {
            return data() + size_;
        }
        [[nodiscard]] constexpr reverse_iterator rbegin() noexcept
        {
            std::make_reverse_iterator(end());
        }
        [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
        {
            std::make_reverse_iterator(end());
        }
        [[nodiscard]] constexpr reverse_iterator rend() noexcept
        {
            std::make_reverse_iterator(begin());
        }
        [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
        {
            std::make_reverse_iterator(begin());
        }
        [[nodiscard]] constexpr const_iterator cbegin() const noexcept
        {
            return begin();
        }
        [[nodiscard]] constexpr const_iterator cend() const noexcept
        {
            return end();
        }
        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
        {
            return rbegin();
        }
        [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
        {
            return rend();
        }

        // 5.5, size/capacity:
        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return size_ == 0;
        }
        [[nodiscard]] constexpr size_type size() const noexcept
        {
            return size_;
        }
        [[nodiscard]] static constexpr size_type max_size() noexcept
        {
            return Capacity;
        }
        [[nodiscard]] static constexpr size_type capacity() noexcept
        {
            return Capacity;
        }
        constexpr void resize(size_type sz)
        {
            static_assert(std::is_default_constructible_v<value_type>, "T must be default constuctible");
            assert(sz <= capacity());
            if (sz < size_)
            {
                const auto amount = size_ - sz;
                std::ranges::destroy_n(data() + sz, amount);
            }
            else
            {
                const auto amount = sz - size_;
                std::ranges::uninitialized_default_construct_n(end(), amount);
            }
            size_ = sz;
        }
        // TODO: constexpr void resize(size_type sz, const value_type& c);

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

        [[nodiscard]] constexpr reference front()
        {
            return *begin();
        }
        [[nodiscard]] constexpr const_reference front() const
        {
            return *begin();
        }

        [[nodiscard]] constexpr reference back()
        {
            return *(begin() + size_ - 1);
        }
        [[nodiscard]] constexpr const_reference back() const
        {
            return *(begin() + size_ - 1);
        }

        [[nodiscard]] constexpr pointer data() noexcept
        {
            return storage_.data();
        }
        [[nodiscard]] constexpr const_pointer data() const noexcept
        {
            return storage_.data();
        }

        // 5.7, modifiers:
        // TODO: constexpr iterator insert(const_iterator position, const value_type& x);
        // TODO: constexpr iterator insert(const_iterator position, value_type&& x);
        // TODO: constexpr iterator insert(const_iterator position, size_type n, const value_type& x);
        // TODO: template <class InputIterator>
        // TODO: constexpr iterator insert(const_iterator position, InputIterator first, InputIterator last);
        // TODO: constexpr iterator insert(const_iterator position, initializer_list<value_type> il);

        // TODO: template <class... Args>
        // TODO: constexpr iterator emplace(const_iterator position, Args&&... args);
        template <class... Args>
        constexpr reference emplace_back(Args&&... args)
        {
            assert(size() < capacity());
            auto* emplaced = std::construct_at(end(), std::forward<value_type>(args)...);
            ++size_;
            return *emplaced;
        }
        constexpr void push_back(const value_type& x)
        {
            emplace_back(x);
        }
        constexpr void push_back(value_type&& x)
        {
            emplace_back(std::move(x));
        }

        constexpr void pop_back()
        {
            std::destroy_at(&back());
            --size_;
        }
        // TODO: constexpr iterator erase(const_iterator position);
        // TODO: constexpr iterator erase(const_iterator first, const_iterator last);

        constexpr void clear() noexcept
        {
            std::ranges::destroy(*this);
            size_ = 0;
        }

        constexpr void swap(static_vector& other) noexcept(std::is_nothrow_swappable_v<value_type>&& std::is_nothrow_move_constructible_v<value_type>) 
            requires std::is_move_constructible_v<value_type> && std::is_swappable_v<value_type>
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
                std::iter_swap(smaller_begin, larger_begin);
            }
            std::uninitialized_move(larger_begin, larger_end, smaller_begin);
            std::swap(size_, other.size_);
        }
    };

    // 5.8, specialized algorithms:
    template <typename T, size_t N>
    constexpr void swap(static_vector<T, N>& x, static_vector<T, N>& y) noexcept(noexcept(x.swap(y)))
    {
        x.swap(y);
    }

}
