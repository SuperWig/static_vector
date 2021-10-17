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

        constexpr static bool is_trivial = std::is_trivial_v<T>;

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
        static_vector(const static_vector& other) requires is_trivial = default;
        static_vector(static_vector&& other) requires is_trivial = default;

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
        static_vector& operator=(const static_vector& other) requires is_trivial = default;
        static_vector& operator=(static_vector&& other) requires is_trivial = default;

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
        constexpr ~static_vector() noexcept requires is_trivial = default;
        constexpr ~static_vector()
        {
            std::ranges::destroy_n(begin(), size_);
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

        [[nodiscard]] constexpr pointer data() noexcept
        {
            return storage_.data();
        }
        [[nodiscard]] constexpr const_pointer data() const noexcept
        {
            return storage_.data();
        }

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
    };
}
