// Copyright (c) Daniel Marshall.
// SPDX-License-Identifier: BSL-1.0

#include <array>
#include <string>
#include <vector>

#include <doctest/doctest.h>
#include <static_vector.h>

// static_assert(std::is_empty_v<dpm::static_vector<int, 0>>);

//TODO: replace std::string usage with a custom type.

struct my_int
{
    int value = 2;
};

struct constructor_count
{
    inline static int count = 0;
    constructor_count() noexcept { ++count; }
    constructor_count(const constructor_count&) noexcept { ++count; }
    constructor_count(constructor_count&&) noexcept { ++count; }
    ~constructor_count() noexcept { --count; }
};

TEST_CASE("constructors/destructors")
{
    SUBCASE("static_vector(consd static_vector&)")
    {
        dpm::static_vector<std::string, 5> sv(2);
        sv[0] = "this is a very long string to bypass SSO.";
        sv[1] = "this is a very long string to bypass SSO.";
        auto copy = sv;
        REQUIRE(copy[0] == sv[0]);
        REQUIRE(copy[1] == sv[1]);
        REQUIRE(&copy[0] != &sv[0]);
        REQUIRE(&copy[1] != &sv[1]);
    }
    SUBCASE("static_vector(static_vector&&)")
    {
        dpm::static_vector<std::string, 2> sv_str(2);
        sv_str[0] = "this is a very long string to bypass SSO.";
        const auto str = sv_str[0].data();
        auto sv_str_moved = std::move(sv_str);
        const auto str2 = sv_str_moved[0].data();
        REQUIRE(str == str2);
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
        dpm::static_vector<std::string, 3> v1{ "this is long string 1.", "this is long string 3." };
        dpm::static_vector<std::string, 3> v2{ "this is long string 2." };
        const auto* test = v1[0].data();
        v1 = v2;
        REQUIRE(v1.size() == 1);
        REQUIRE(v1[0].data() == test);
        REQUIRE(v1[0] == "this is long string 2.");
        REQUIRE(v2[0] == "this is long string 2.");
        REQUIRE(v1 == v2);
    }
    SUBCASE("move")
    {
        dpm::static_vector<std::string, 3> v1{ "this is long string 1." };
        dpm::static_vector<std::string, 3> v2{ "this is long string 2.", "this is long string 3." };
        const auto* test = v2[0].data();
        v1 = std::move(v2);
        REQUIRE(v1.size() == 2);
        REQUIRE(v1[0].data() == test);
        REQUIRE(v2.empty());
    }
    SUBCASE("assign(first, last)")
    {
        dpm::static_vector<std::string, 3> v1{ "I", "am", "Dan" };
        dpm::static_vector<std::string, 3> v2{ "hello" };
        v1.assign(v2.begin(), v2.end());
    }
    SUBCASE("assign(n, value)")
    {
        dpm::static_vector<std::string, 3> v1{ "I", "am"};
        v1.assign(3, "Good");
        v1.clear();
        v1.assign(3, "Bad");
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

    dpm::static_vector<std::string, 3> bob{ "this is a long string 1", "this is a long string 2", "this is a long string 3" };
    bob.resize(1);
    bob.resize(3);
    dpm::static_vector<std::string, 3> bill;
    bill.resize(2);

    bill.clear();
    bill.resize(3, "Yo");

    REQUIRE(bill.size() == 3);
    REQUIRE(bill[0] == "Yo");
    REQUIRE(bill[1] == "Yo");
    REQUIRE(bill[2] == "Yo");
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
    //TODO: actually test these
    SUBCASE("emplace_back")
    {
        dpm::static_vector<std::string, 3> v1;
        v1.emplace_back("hello");
        v1.emplace_back("world");
        v1.emplace_back(3, 'c');
    }
    SUBCASE("push_back")
    {
        dpm::static_vector<std::string, 3> v1;
        v1.push_back("hello");

        std::string world = "world and lets disable SSO.";
        auto original = world.data();
        v1.push_back(std::move(world));
        REQUIRE(original == v1[1].data());
    }
    SUBCASE("pop_back")
    {
        dpm::static_vector<std::string, 3> v1;
        v1.push_back("hello this is a long string.");
        v1.pop_back();
        v1.push_back("hello");
    }
    SUBCASE("swap")
    {
        {
            dpm::static_vector<std::string, 3> v1{ "this is a test, homes." };
            dpm::static_vector<std::string, 3> v2{ "Bob is so very cool.", "Bob is the best homie" };

            auto v10 = v1[0].data();
            auto v20 = v2[0].data();
            auto v21 = v2[1].data();

            v1.swap(v2);
            REQUIRE(v1.size() == 2);
            REQUIRE(v2.size() == 1);

            REQUIRE(v1[0] == v20);
            REQUIRE(v1[1] == v21);
            REQUIRE(v2[0] == v10);
        }
        {
            dpm::static_vector<std::string, 3> v1{ "Bob is so very cool.", "Bob is the best homie" };
            dpm::static_vector<std::string, 3> v2{ "this is a test, homes." };

            auto v10 = v1[0].data();
            auto v11 = v1[1].data();
            auto v20 = v2[0].data();

            v1.swap(v2);

            REQUIRE(v1.size() == 1);
            REQUIRE(v2.size() == 2);

            REQUIRE(v1[0] == v20);
            REQUIRE(v2[0] == v10);
            REQUIRE(v2[1] == v11);
        }
        {
            dpm::static_vector<std::string, 3> v1{ "Bob is so very cool.", "Bob is the best homie" };
            dpm::static_vector<std::string, 3> v2{ "this is a test, homes.", "just a long string" };

            auto v10 = v1[0].data();
            auto v11 = v1[1].data();
            auto v20 = v2[0].data();
            auto v21 = v2[1].data();

            v1.swap(v2);

            REQUIRE(v1.size() == 2);
            REQUIRE(v2.size() == 2);

            REQUIRE(v1[0] == v20);
            REQUIRE(v1[1] == v21);
            REQUIRE(v2[0] == v10);
            REQUIRE(v2[1] == v11);
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
            dpm::static_vector<std::string, 5> v1{ "hello", "world" };
            std::array<std::string, 3> str_arr{ "this", "is", "cool" };
            auto inserted = v1.insert(v1.begin() + 1, str_arr.begin(), str_arr.end());
            REQUIRE(v1.size() == 5);
            REQUIRE(inserted == &v1[1]);
        }
    }
    SUBCASE("erase")
    {
        {
            dpm::static_vector<std::string, 3> vec{ "this", "is", "bob" };
            vec.erase(vec.begin() + 1);
            REQUIRE(vec.size() == 2);
            REQUIRE(vec[0] == "this");
            REQUIRE(vec[1] == "bob");
        }
        {
            dpm::static_vector<std::string, 3> vec{ "this", "is", "bob" };
            vec.erase(vec.begin(), vec.begin() + 2);
            REQUIRE(vec.size() == 1);
            REQUIRE(vec[0] == "bob");
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
