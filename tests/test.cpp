#include <array>
#include <string>
#include <vector>

#include <doctest/doctest.h>
#include <static_vector.h>

// static_assert(std::is_empty_v<dpm::static_vector<int, 0>>);

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
}

TEST_CASE("ranges")
{
    dpm::static_vector<int, 5> test(5, 2);
    auto begin = test.begin();
    auto end = test.end();
    for (auto i : test)
    {
        REQUIRE(i == 2);
    }
}