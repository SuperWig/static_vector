// Copyright (c) Daniel Marshall.
// SPDX-License-Identifier: BSL-1.0

#include <array>
#include <string>
#include <vector>

#include <doctest/doctest.h>
#include <static_vector.h>

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
        dpm::static_vector<constructor_count, 5> sv2(0);
        REQUIRE(constructor_count::count == 0);
        dpm::static_vector<constructor_count, 5> sv1(3);
        REQUIRE(constructor_count::count == 3);
    }
    SUBCASE("static_vector(size_type, value_type)")
    {
        dpm::static_vector<int, 3> test(3, 2);
        REQUIRE(test.size() == 3);
        REQUIRE(test[0] == 2);
        REQUIRE(test[1] == 2);
        REQUIRE(test[2] == 2);
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
            dpm::static_vector<constructor_count, 3> test1(3);
            dpm::static_vector<constructor_count, 3> test2(1);
        }
        REQUIRE(constructor_count::count == 0);
    }
}

TEST_CASE("assignment")
{
    SUBCASE("copy")
    {
         dpm::static_vector<copy_move_tester, 3> v1{ 1, 3 };
         dpm::static_vector<copy_move_tester, 3> v2{ 2 };
         const auto* test = v1[0].addr();
         v1 = v2;
         REQUIRE(v1.size() == 1);
         REQUIRE(v1[0].addr() == test);
         REQUIRE(v1[0].value() == 2);
         REQUIRE(v2[0].value() == 2);
         //REQUIRE(v1 == v2);
    }
    SUBCASE("move")
    {
         dpm::static_vector<copy_move_tester, 3> v1{ 1 };
         dpm::static_vector<copy_move_tester, 3> v2{ 2, 3 };
         const auto* test = v2[0].addr();
         v1 = std::move(v2);
         REQUIRE(v1.size() == 2);
         REQUIRE(v1[0].addr() == test);
         REQUIRE(v2.empty());
    }
    SUBCASE("assign(first, last)")
    {
        dpm::static_vector<copy_move_tester, 3> v1{ 1, 2, 3 };
        dpm::static_vector<copy_move_tester, 3> v2{ 4 };
        v1.assign(v2.begin(), v2.end());
    }
    SUBCASE("assign(n, value)")
    {
        dpm::static_vector<copy_move_tester, 3> v1{ 1, 2 };
        v1.assign(3, 10);
        v1.clear();
        v1.assign(3, 20);
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

    dpm::static_vector<copy_move_tester, 3> bob{ 1, 2, 3 };
    bob.resize(1);
    bob.resize(3);
    dpm::static_vector<copy_move_tester, 3> bill;
    bill.resize(2);

    bill.clear();
    bill.resize(3, 30);

    REQUIRE(bill.size() == 3);
    REQUIRE(bill[0].value() == 30);
    REQUIRE(bill[1].value() == 30);
    REQUIRE(bill[2].value() == 30);
}

TEST_CASE("access")
{
    dpm::static_vector<int, 3> test{ 1, 2, 3 };
    REQUIRE(test.front() == 1);
    REQUIRE(std::is_same_v<decltype(test.front()), int&>);

    REQUIRE(test.back() == 3);
    REQUIRE(std::is_same_v<decltype(test.back()), int&>);

    const dpm::static_vector<int, 3> const_test{ 1, 2, 3 };
    REQUIRE(const_test.front() == 1);
    REQUIRE(std::is_same_v<decltype(const_test.front()), const int&>);

    REQUIRE(const_test.back() == 3);
    REQUIRE(std::is_same_v<decltype(const_test.back()), const int&>);
}

TEST_CASE("modifiers")
{
    SUBCASE("clear")
    {
        dpm::static_vector<constructor_count, 3> test(3);
        REQUIRE(!test.empty());
        REQUIRE(constructor_count::count > 0);
        test.clear();
        REQUIRE(test.empty());
        REQUIRE(constructor_count::count == 0);
    }
    // TODO: actually test these
    SUBCASE("emplace_back")
    {
        dpm::static_vector<std::string, 3> v1;
        v1.emplace_back("hello");
        v1.emplace_back("world");
        v1.emplace_back(3, 'c');
    }
    SUBCASE("push_back")
    {
        dpm::static_vector<copy_move_tester, 3> v1;
        v1.push_back(20);

        copy_move_tester world = 42;
        auto original = world.addr();
        v1.push_back(std::move(world));
        REQUIRE(original == v1[1].addr());
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
            dpm::static_vector<copy_move_tester, 3> v1{ 1 };
            dpm::static_vector<copy_move_tester, 3> v2{ 2, 3 };

            auto v10 = v1[0].value();
            auto v20 = v2[0].value();
            auto v21 = v2[1].value();

            v1.swap(v2);
            REQUIRE(v1.size() == 2);
            REQUIRE(v2.size() == 1);

            REQUIRE(v1[0].value() == v20);
            REQUIRE(v1[1].value() == v21);
            REQUIRE(v2[0].value() == v10);
        }
        {
            dpm::static_vector<copy_move_tester, 3> v1{ 1, 2 };
            dpm::static_vector<copy_move_tester, 3> v2{ 3 };

            auto v10 = v1[0].value();
            auto v11 = v1[1].value();
            auto v20 = v2[0].value();

            v1.swap(v2);

            REQUIRE(v1.size() == 1);
            REQUIRE(v2.size() == 2);

            REQUIRE(v1[0].value() == v20);
            REQUIRE(v2[0].value() == v10);
            REQUIRE(v2[1].value() == v11);
        }
        {
            dpm::static_vector<copy_move_tester, 3> v1{ 1, 3 };
            dpm::static_vector<copy_move_tester, 3> v2{ 2, 4 };

            auto v10 = v1[0].value();
            auto v11 = v1[1].value();
            auto v20 = v2[0].value();
            auto v21 = v2[1].value();

            v1.swap(v2);

            REQUIRE(v1.size() == 2);
            REQUIRE(v2.size() == 2);

            REQUIRE(v1[0].value() == v20);
            REQUIRE(v1[1].value() == v21);
            REQUIRE(v2[0].value() == v10);
            REQUIRE(v2[1].value() == v11);
        }
    }
    SUBCASE("insert")
    {
        {
            dpm::static_vector<int, 5> v1{ 1, 2, 3 };
            auto inserted = v1.insert(v1.begin() + 1, 5);
            REQUIRE(v1.size() == 4);
            REQUIRE(*inserted == 5);
        }
        {
            dpm::static_vector<int, 5> v1{ 1, 2, 3 };
            auto inserted = v1.insert(v1.begin(), 2, 7);
            REQUIRE(v1.size() == 5);
            REQUIRE(inserted == &v1[0]);
        }
        {
            dpm::static_vector<copy_move_tester, 5> v1{ 1, 2 };
            std::array<copy_move_tester, 3> str_arr{ 3, 4, 5 };
            auto inserted = v1.insert(v1.begin() + 1, str_arr.begin(), str_arr.end());
            REQUIRE(v1.size() == 5);
            REQUIRE(inserted == &v1[1]);
        }
    }
    SUBCASE("erase")
    {
        {
            dpm::static_vector<copy_move_tester, 3> vec{ 1, 2, 3 };
            vec.erase(vec.begin() + 1);
            REQUIRE(vec.size() == 2);
            REQUIRE(vec[0].value() == 1);
            REQUIRE(vec[1].value() == 3);
        }
        {
            dpm::static_vector<copy_move_tester, 3> vec{ 1, 2, 3 };
            vec.erase(vec.begin(), vec.begin() + 2);
            REQUIRE(vec.size() == 1);
            REQUIRE(vec[0].value() == 3);
            vec.erase(vec.begin(), vec.begin());
        }
    }
}


TEST_CASE("ranges")
{
    SUBCASE("range-based for loop")
    {
        dpm::static_vector<int, 5> test(5, 2);
        auto begin = test.begin();
        auto end = test.end();
        for (auto i : test)
        {
            REQUIRE(i == 2);
        }
    }
}
