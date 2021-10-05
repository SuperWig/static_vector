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


TEST_CASE("constructors")
{
    SUBCASE("static_vector(consd static_vector&)")
    {
        dpm::static_vector<std::string, 5> sv(2);
        sv[0] = "long string to bypass SSO";
        sv[1] = "long string to bypass SSO";
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
        {
            dpm::static_vector<constructor_count, 5> sv1(3);
            REQUIRE(constructor_count::count == 3);
        }
        {
            dpm::static_vector<constructor_count, 5> sv2(0);
            REQUIRE(constructor_count::count == 0);
        }
    }
    SUBCASE("static_vector(size_type, value_type)")
    {
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
            dpm::static_vector<constructor_count, 1> test(1);
        }
        REQUIRE(constructor_count::count == 0);
    }
}