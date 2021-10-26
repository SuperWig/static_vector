// Copyright (c) Daniel Marshall.
// SPDX-License-Identifier: BSL-1.0

#include <array>
#include <string>
#include <vector>

#include <doctest/doctest.h>
#include <dpm/static_vector.h>

// static_assert(std::is_empty_v<dpm::static_vector<int, 0>>);

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

TEST_CASE("constructors/destructors")
{
    SUBCASE("static_vector(const static_vector&)")
    {
        dpm::static_vector<copy_move_tester, 5> sv(2);
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
        dpm::static_vector<copy_move_tester, 2> sv(2);
        const auto address = sv[0].addr();
        auto moved_sv = std::move(sv);
        REQUIRE(address == moved_sv[0].addr());
    }
    SUBCASE("static_vector(size_type)")
    {
        dpm::static_vector<constructor_count, 5> sv1(0);
        REQUIRE(constructor_count::count == 0);
        dpm::static_vector<constructor_count, 5> sv2(3);
        REQUIRE(constructor_count::count == 3);
    }
    SUBCASE("static_vector(size_type, value_type)")
    {
        dpm::static_vector<int, 3> sv(3, 2);
        REQUIRE(sv.size() == 3);
        REQUIRE(sv[0] == 2);
        REQUIRE(sv[1] == 2);
        REQUIRE(sv[2] == 2);
    }
    SUBCASE("static_vector(InputIter, InputIter)")
    {
        std::array<int, 3> test{ 1, 2, 3 };
        dpm::static_vector<int, 3> sv(test.begin(), test.end());

        REQUIRE(sv[0] == 1);
        REQUIRE(sv[1] == 2);
        REQUIRE(sv[2] == 3);
    }
    SUBCASE("~static_vector()")
    {
        {
            dpm::static_vector<constructor_count, 3> sv1(3);
            dpm::static_vector<constructor_count, 3> sv2(1);
        }
        REQUIRE(constructor_count::count == 0);
    }
}

TEST_CASE("assignment")
{
    SUBCASE("copy")
    {
         dpm::static_vector<copy_move_tester, 3> sv1{ 1, 3 };
         dpm::static_vector<copy_move_tester, 3> sv2{ 2 };
         const auto* sv1_0 = sv1[0].addr();
         sv1 = sv2;
         REQUIRE(sv1.size() == 1);
         REQUIRE(sv1[0].addr() == sv1_0);
         REQUIRE(sv1[0].value() == 2);
         REQUIRE(sv2[0].value() == 2);
         //REQUIRE(v1 == v2);
    }
    SUBCASE("move")
    {
         dpm::static_vector<copy_move_tester, 3> sv1{ 1 };
         dpm::static_vector<copy_move_tester, 3> sv2{ 2, 3 };
         const auto* sv2_0 = sv2[0].addr();
         sv1 = std::move(sv2);
         REQUIRE(sv1.size() == 2);
         REQUIRE(sv1[0].addr() == sv2_0);
         REQUIRE(sv2.empty());
    }
    SUBCASE("assign(first, last)")
    {
        dpm::static_vector<copy_move_tester, 3> sv1{ 1, 2, 3 };
        dpm::static_vector<copy_move_tester, 3> sv2{ 4 };
        sv1.assign(sv2.begin(), sv2.end());
    }
    SUBCASE("assign(n, value)")
    {
        dpm::static_vector<copy_move_tester, 3> sv1{ 1, 2 };
        sv1.assign(3, 10);
        sv1.clear();
        sv1.assign(3, 20);
    }
}

TEST_CASE("size/capacity")
{
    dpm::static_vector<int, 2> sv1;
    dpm::static_vector<int, 2> sv2(2);
    REQUIRE(sv1.size() == 0);
    REQUIRE(sv2.size() == 2);
    REQUIRE(sv1.empty());
    REQUIRE(!sv2.empty());
    REQUIRE(sv1.capacity() == 2);
    REQUIRE(sv2.capacity() == 2);
    REQUIRE(sv1.max_size() == sv1.capacity());

    dpm::static_vector<copy_move_tester, 3> sv3{ 1, 2, 3 };
    sv3.resize(1);
    sv3.resize(3);
    dpm::static_vector<copy_move_tester, 3> sv4;
    sv4.resize(2);

    sv4.clear();
    sv4.resize(3, 30);

    REQUIRE(sv4.size() == 3);
    REQUIRE(sv4[0].value() == 30);
    REQUIRE(sv4[1].value() == 30);
    REQUIRE(sv4[2].value() == 30);
}

TEST_CASE("access")
{
    dpm::static_vector<int, 3> sv{ 1, 2, 3 };
    REQUIRE(sv.front() == 1);
    REQUIRE(std::is_same_v<decltype(sv.front()), int&>);

    REQUIRE(sv.back() == 3);
    REQUIRE(std::is_same_v<decltype(sv.back()), int&>);

    const dpm::static_vector<int, 3> const_sv{ 1, 2, 3 };
    REQUIRE(const_sv.front() == 1);
    REQUIRE(std::is_same_v<decltype(const_sv.front()), const int&>);

    REQUIRE(const_sv.back() == 3);
    REQUIRE(std::is_same_v<decltype(const_sv.back()), const int&>);
}

TEST_CASE("modifiers")
{
    SUBCASE("clear")
    {
        dpm::static_vector<constructor_count, 3> sv(3);
        REQUIRE(!sv.empty());
        REQUIRE(constructor_count::count > 0);
        sv.clear();
        REQUIRE(sv.empty());
        REQUIRE(constructor_count::count == 0);
    }
    // TODO: actually test these
    SUBCASE("emplace_back")
    {
        dpm::static_vector<std::string, 3> sv;
        sv.emplace_back("hello");
        sv.emplace_back("world");
        sv.emplace_back(3, 'c');
    }
    SUBCASE("push_back")
    {
        dpm::static_vector<copy_move_tester, 3> sv;
        sv.push_back(20);

        copy_move_tester value = 42;
        auto original = value.addr();
        sv.push_back(std::move(value));
        REQUIRE(original == sv[1].addr());
    }
    SUBCASE("pop_back")
    {
        dpm::static_vector<copy_move_tester, 3> v1;
        v1.push_back(20);
        v1.pop_back();
        v1.push_back(10);
    }
    SUBCASE("swap")
    {
        {
            dpm::static_vector<copy_move_tester, 3> sv1{ 1 };
            dpm::static_vector<copy_move_tester, 3> sv2{ 2, 3 };

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
            dpm::static_vector<copy_move_tester, 3> sv1{ 1, 2 };
            dpm::static_vector<copy_move_tester, 3> sv2{ 3 };

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
            dpm::static_vector<copy_move_tester, 3> sv1{ 1, 3 };
            dpm::static_vector<copy_move_tester, 3> sv2{ 2, 4 };

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
            dpm::static_vector<int, 5> sv{ 1, 2, 3 };
            auto inserted = sv.insert(sv.begin() + 1, 5);
            REQUIRE(sv.size() == 4);
            REQUIRE(*inserted == 5);
        }
        {
            dpm::static_vector<int, 5> sv{ 1, 2, 3 };
            auto inserted = sv.insert(sv.begin(), 2, 7);
            REQUIRE(sv.size() == 5);
            REQUIRE(inserted == &sv[0]);
        }
        {
            dpm::static_vector<copy_move_tester, 5> sv{ 1, 2 };
            std::array<copy_move_tester, 3> arr{ 3, 4, 5 };
            auto inserted = sv.insert(sv.begin() + 1, arr.begin(), arr.end());
            REQUIRE(sv.size() == 5);
            REQUIRE(inserted == &sv[1]);
        }
    }
    SUBCASE("erase")
    {
        {
            dpm::static_vector<copy_move_tester, 3> sv{ 1, 2, 3 };
            sv.erase(sv.begin() + 1);
            REQUIRE(sv.size() == 2);
            REQUIRE(sv[0].value() == 1);
            REQUIRE(sv[1].value() == 3);
        }
        {
            dpm::static_vector<copy_move_tester, 3> sv{ 1, 2, 3 };
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
        dpm::static_vector<int, 5> sv(5, 2);
        auto begin = sv.begin();
        auto end = sv.end();
        for (auto i : sv)
        {
            REQUIRE(i == 2);
        }
    }
}
