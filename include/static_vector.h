#pragma once

// #include <algorithm>
#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>

namespace dpm
{
    template <class T>
    struct aligned_storage
    {
        alignas(T) std::byte storage[sizeof(T)];

        T* data()
        {
            return reinterpret_cast<T*>(&storage);
        }
        const T* data() const
        {
            return reinterpret_cast<T*>(&storage);
        }
    };
    template <class T, std::size_t Capacity>
    class static_vector
    {
        alignas(T) aligned_storage<T> storage_[Capacity];
        std::size_t size_ = 0;

    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = value_type&;
        using const_reference = const value_type&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        // using iterator = implementation - defined;       // see [container.requirements]
        // using const_iterator = implementation - defined; // see [container.requirements]
        // using reverse_iterator = std::reverse_iterator<iterator>;
        // using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // 5.2, copy/move construction:
        constexpr static_vector() noexcept = default;
        constexpr static_vector(const static_vector& other) : size_(other.size_)
        {
            for (size_t i = 0; i < size_; ++i)
            {
                std::construct_at(&get(i), other[i]);
            }
        }
        constexpr static_vector(static_vector&& other) noexcept : size_(std::exchange(other.size_, 0))
        {
            static_assert(std::is_move_constructible_v<value_type>, "value_type must be move constructible.");
            for (size_t i = 0; i < size_; ++i)
            {
                std::construct_at(&get(i), std::move(other.get(i)));
            }
        }
        constexpr explicit static_vector(size_type count) : size_(count)
        {
            static_assert(std::is_default_constructible_v<value_type>, "value_type must be default constructible.");
            assert(count <= capacity());
            for (size_t i = 0; i < count; ++i)
            {
                std::construct_at(&get(i));
            }
        }
        constexpr explicit static_vector(size_type count, const value_type& value)
        {
            static_assert(std::is_copy_constructible_v<value_type>, "value_type must be copy constructible.");
            assert(count <= capacity());
            for (size_t i = 0; i < count; ++i)
            {
                std::construct_at(&get(i), value);
            }
        }
        template <std::input_iterator InputIter>
        constexpr static_vector(InputIter first, InputIter last)
        {
            static_assert(std::is_constructible_v<value_type, decltype(*first)>, "value_type must be constructible from decltype(*first)");
            size_ = std::distance(first, last);
            assert(size_ <= capacity());

            for (size_t i = 0; i < size_; ++i, ++first)
            {
                std::construct_at(&get(i), *first);
            }
        }
        constexpr static_vector(std::initializer_list<value_type> il) : size_(il.size())
        {
            auto first = il.begin();
            for (size_t i = 0; i < size_; ++i, ++first)
            {
                std::construct_at(&get(i), *first);
            }
        }
        // 5.3, copy/move assignment:
        constexpr static_vector& operator=(const static_vector& other) noexcept(std::is_nothrow_copy_assignable_v<value_type>);
        constexpr static_vector& operator=(static_vector&& other) noexcept(std::is_nothrow_move_assignable_v<value_type>);
        template <class InputIterator>
        constexpr void assign(InputIterator first, InputIterator last);
        constexpr void assign(size_type n, const value_type& u);
        constexpr void assign(std::initializer_list<value_type> il);

        // 5.4, destruction
        constexpr ~static_vector() noexcept
        {
            for (size_t i = 0; i < size_; i++)
            {
                std::destroy_at(&get(0));
            }
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

        reference operator[](size_t n) noexcept
        {
            assert(n < size_ && n >= 0);
            return get(n);
        }
        const_reference operator[](size_t n) const noexcept
        {
            assert(n < size_ && n >= 0);
            return get(n);
        }

    private:
        reference get(size_t n) noexcept
        {
            return reinterpret_cast<reference>(storage_[n]);
        }
        const_reference get(size_t n) const noexcept
        {
            return reinterpret_cast<const_reference>(storage_[n]);
        }
    };
}