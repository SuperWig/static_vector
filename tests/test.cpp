// Copyright (c) Daniel Marshall.
// SPDX-License-Identifier: BSL-1.0

#include <array>
#include <string>
#include <vector>

#include <doctest/doctest.h>
#include <dpm/static_vector.h>

using namespace dpm;

// static_assert(std::is_empty_v<static_vector<int, 0>>);

struct constructor_count
{
    inline static int count = 0;
    constructor_count() noexcept { ++count; }
    constructor_count(const constructor_count&) noexcept { ++count; }
    constructor_count(constructor_count&&) noexcept { ++count; }
    ~constructor_count() noexcept { --count; }
};

struct copy_move_tester
{
    int* data = new int(1);

    copy_move_tester() = default;
    copy_move_tester(int value) : data(new int(value)) {}
    copy_move_tester(const copy_move_tester& other) : data(new int(*other.data)) {}
    copy_move_tester(copy_move_tester&& other) : data(std::exchange(other.data, nullptr)) {}

    copy_move_tester& operator=(int value)
    {
        *data = value;
        return *this;
    }
    copy_move_tester& operator=(const copy_move_tester& other)
    {
        *data = *other.data;
        return *this;
    }
    copy_move_tester& operator=(copy_move_tester&& other)
    {
        data = std::exchange(other.data, nullptr);
        return *this;
    }

    ~copy_move_tester() { delete data; }

    int value() const { return *data; }
    const int* addr() const { return data; }
};

struct copy_only
{
    copy_only() = default;
    copy_only(const copy_only&) = default;
    copy_only(copy_only&&) = delete;
};

struct move_only
{
    move_only() = default;
    move_only(const move_only&) = delete;
    move_only(move_only&&) = default;
};

TEST_CASE("constructors/destructors")
{
    SUBCASE("static_vector()")
    {
        static_vector<copy_move_tester, 5> sv1;
        static_vector<int, 5> sv2;

        REQUIRE(sv1.empty());
        REQUIRE(sv2.empty());
    }
    SUBCASE("static_vector(const static_vector&)")
    {
        static_vector<copy_move_tester, 5> sv(2);
        sv[0] = 10;
        sv[1] = 20;
        auto copy = sv;

        REQUIRE(copy[0].value() == sv[0].value());
        REQUIRE(copy[1].value() == sv[1].value());
        REQUIRE(copy[0].addr() != sv[0].addr());
        REQUIRE(copy[1].addr() != sv[1].addr());
    }
    SUBCASE("static_vector(static_vector&&)")
    {
        static_vector<copy_move_tester, 2> sv(2);
        const auto sv_0 = sv[0].addr();
        const auto sv_1 = sv[1].addr();
        auto moved_sv = std::move(sv);

        REQUIRE(sv_0 == moved_sv[0].addr());
        REQUIRE(sv_1 == moved_sv[1].addr());
    }
    SUBCASE("static_vector(size_type)")
    {
        static_vector<constructor_count, 5> sv1(0);
        REQUIRE(constructor_count::count == 0);

        static_vector<constructor_count, 5> sv2(3);
        REQUIRE(constructor_count::count == 3);
    }
    SUBCASE("static_vector(size_type, value_type)")
    {
        static_vector<int, 3> sv(3, 2);

        REQUIRE(sv.size() == 3);
        REQUIRE(sv[0] == 2);
        REQUIRE(sv[1] == 2);
        REQUIRE(sv[2] == 2);
    }
    SUBCASE("static_vector(InputIter, InputIter)")
    {
        std::array<int, 3> test{ 1, 2, 3 };
        static_vector<int, 3> sv(test.begin(), test.end());

        REQUIRE(sv.size() == 3);
        REQUIRE(sv[0] == 1);
        REQUIRE(sv[1] == 2);
        REQUIRE(sv[2] == 3);
    }
    SUBCASE("~static_vector()")
    {
        {
            static_vector<constructor_count, 3> sv1(3);
            static_vector<constructor_count, 3> sv2(1);
        }
        REQUIRE(constructor_count::count == 0);
    }
}

TEST_CASE("assignment")
{
    SUBCASE("copy")
    {
         static_vector<copy_move_tester, 3> sv1{ 1, 3 };
         static_vector<copy_move_tester, 3> sv2{ 2 };
         const auto* sv1_0 = sv1[0].addr();
         sv1 = sv2;
         
         REQUIRE(sv1.size() == 1);
         REQUIRE(sv2.size() == 1);
         REQUIRE(sv1[0].addr() == sv1_0);
         REQUIRE(sv1[0].value() == 2);
         REQUIRE(sv2[0].value() == 2);
    }
    SUBCASE("move")
    {
         static_vector<copy_move_tester, 3> sv1{ 1 };
         static_vector<copy_move_tester, 3> sv2{ 2, 3 };
         const auto* sv2_0 = sv2[0].addr();
         sv1 = std::move(sv2);
        
         REQUIRE(sv1.size() == 2);
         REQUIRE(sv2.size() == 0);
         REQUIRE(sv1[0].addr() == sv2_0);
         REQUIRE(sv2.empty());
    }
    SUBCASE("assign(first, last)")
    {
        static_vector<copy_move_tester, 3> sv1{ 1, 2, 3 };
        static_vector<copy_move_tester, 3> sv2{ 4 };
        sv1.assign(sv2.begin(), sv2.end());
        
        REQUIRE(sv1.size() == 1);
        REQUIRE(sv2.size() == 1);
        REQUIRE(sv1[0].value() == sv2[0].value());
    }
    SUBCASE("assign(n, value)")
    {
        static_vector<copy_move_tester, 3> sv{ 1, 2 };
        auto sv_0 = sv[0].addr();
        auto sv_1 = sv[1].addr();

        sv.assign(3, 10);

        REQUIRE(sv.size() == 3);
        REQUIRE(sv[0].addr() == sv_0);
        REQUIRE(sv[1].addr() == sv_1);
        REQUIRE(sv[0].value() == 10);
        REQUIRE(sv[1].value() == 10);
        REQUIRE(sv[2].value() == 10);
    }
}

TEST_CASE("size/capacity")
{
    static_vector<int, 2> sv1;
    static_vector<int, 2> sv2(2);
    REQUIRE(sv1.size() == 0);
    REQUIRE(sv2.size() == 2);
    REQUIRE(sv1.empty());
    REQUIRE(!sv2.empty());
    REQUIRE(sv1.capacity() == 2);
    REQUIRE(sv2.capacity() == 2);
    REQUIRE(sv1.max_size() == sv1.capacity());

    static_vector<constructor_count, 3> sv3(3);
    REQUIRE(constructor_count::count == 3);
    sv3.resize(2);
    REQUIRE(constructor_count::count == 2);
    sv3.resize(0);
    REQUIRE(constructor_count::count == 0);

}

TEST_CASE("access")
{
    static_vector<int, 3> sv{ 1, 2, 3 };
    REQUIRE(sv.front() == 1);
    REQUIRE(std::is_same_v<decltype(sv.front()), int&>);

    REQUIRE(sv.back() == 3);
    REQUIRE(std::is_same_v<decltype(sv.back()), int&>);

    const static_vector<int, 3> const_sv{ 1, 2, 3 };
    REQUIRE(const_sv.front() == 1);
    REQUIRE(std::is_same_v<decltype(const_sv.front()), const int&>);

    REQUIRE(const_sv.back() == 3);
    REQUIRE(std::is_same_v<decltype(const_sv.back()), const int&>);
}

TEST_CASE("modifiers")
{
    SUBCASE("clear")
    {
        static_vector<constructor_count, 3> sv(3);
        REQUIRE(!sv.empty());
        REQUIRE(constructor_count::count > 0);
        sv.clear();
        REQUIRE(sv.empty());
        REQUIRE(constructor_count::count == 0);
    }
    SUBCASE("emplace_back")
    {
        static_vector<std::string, 3> sv;
        decltype(auto) emplaced = sv.emplace_back("hello");

        REQUIRE(std::is_same_v<decltype(emplaced), std::string&>);
        REQUIRE(emplaced == "hello");
        REQUIRE(sv.size() == 1);
    }
    SUBCASE("push_back")
    {
        static_vector<copy_move_tester, 3> sv;
        sv.push_back(20);

        copy_move_tester value = 42;
        auto original = value.addr();
        sv.push_back(std::move(value));
        REQUIRE(original == sv[1].addr());
    }
    SUBCASE("pop_back")
    {
        static_vector<int, 3> sv;
        sv.push_back(10);
        sv.push_back(20);
        REQUIRE(sv.size() == 2);
        REQUIRE(sv.back() == 20);
        sv.pop_back();
        REQUIRE(sv.size() == 1);
        REQUIRE(sv.back() == 10);

    }
    SUBCASE("swap")
    {
        {
            static_vector<copy_move_tester, 3> sv1{ 1 };
            static_vector<copy_move_tester, 3> sv2{ 2, 3 };

            auto sv1_0 = sv1[0].value();
            auto sv2_0 = sv2[0].value();
            auto sv2_1 = sv2[1].value();

            sv1.swap(sv2);
            REQUIRE(sv1.size() == 2);
            REQUIRE(sv2.size() == 1);

            REQUIRE(sv1[0].value() == sv2_0);
            REQUIRE(sv1[1].value() == sv2_1);
            REQUIRE(sv2[0].value() == sv1_0);
        }
        {
            static_vector<copy_move_tester, 3> sv1{ 1, 2 };
            static_vector<copy_move_tester, 3> sv2{ 3 };

            auto sv1_0 = sv1[0].value();
            auto sv1_1 = sv1[1].value();
            auto sv2_0 = sv2[0].value();

            swap(sv1,sv2);

            REQUIRE(sv1.size() == 1);
            REQUIRE(sv2.size() == 2);

            REQUIRE(sv1[0].value() == sv2_0);
            REQUIRE(sv2[0].value() == sv1_0);
            REQUIRE(sv2[1].value() == sv1_1);
        }
        {
            static_vector<copy_move_tester, 3> sv1{ 1, 3 };
            static_vector<copy_move_tester, 3> sv2{ 2, 4 };

            auto sv1_0 = sv1[0].value();
            auto sv1_1 = sv1[1].value();
            auto sv2_0 = sv2[0].value();
            auto sv2_1 = sv2[1].value();

            sv1.swap(sv2);

            REQUIRE(sv1.size() == 2);
            REQUIRE(sv2.size() == 2);

            REQUIRE(sv1[0].value() == sv2_0);
            REQUIRE(sv1[1].value() == sv2_1);
            REQUIRE(sv2[0].value() == sv1_0);
            REQUIRE(sv2[1].value() == sv1_1);
        }
    }
    SUBCASE("insert")
    {
        {
            static_vector<int, 5> sv{ 1, 2, 3 };
            auto inserted = sv.insert(sv.begin() + 1, 5);
            REQUIRE(sv.size() == 4);
            REQUIRE(*inserted == 5);
        }
        {
            static_vector<int, 5> sv{ 1, 2, 3 };
            auto inserted = sv.insert(sv.begin(), 2, 7);
            REQUIRE(sv.size() == 5);
            REQUIRE(inserted == &sv[0]);
        }
        {
            static_vector<copy_move_tester, 5> sv{ 1, 2 };
            std::array<copy_move_tester, 3> arr{ 3, 4, 5 };
            auto inserted = sv.insert(sv.begin() + 1, arr.begin(), arr.end());
            REQUIRE(sv.size() == 5);
            REQUIRE(inserted == &sv[1]);
        }
    }
    SUBCASE("erase")
    {
        {
            static_vector<copy_move_tester, 3> sv{ 1, 2, 3 };
            sv.erase(sv.begin() + 1);
            REQUIRE(sv.size() == 2);
            REQUIRE(sv[0].value() == 1);
            REQUIRE(sv[1].value() == 3);
        }
        {
            static_vector<copy_move_tester, 3> sv{ 1, 2, 3 };
            sv.erase(sv.begin(), sv.begin() + 2);
            REQUIRE(sv.size() == 1);
            REQUIRE(sv[0].value() == 3);
            sv.erase(sv.begin(), sv.begin());
        }
    }
}


TEST_CASE("ranges")
{
    SUBCASE("range-based for loop")
    {
        static_vector<int, 5> sv(5, 2);
        auto begin = sv.begin();
        auto end = sv.end();
        for (auto i : sv)
        {
            REQUIRE(i == 2);
        }
    }
}
