#include "static_vector.h"

#include <string>
#include <type_traits>

using trivial_vector = dpm::static_vector<int, 2>;
using non_trivial_vector = dpm::static_vector<std::string, 2>;

//static_assert(std::is_trivial_v<trivial_vector>, "trivial_vector isn't trivial.");
//static_assert(!std::is_trivial_v<non_trivial_vector>, "non_trivial_vector is trivial.");

//static_assert(std::is_trivially_constructible_v<trivial_vector>, "trivial_vector isn't trivially default constructible.");
//static_assert(!std::is_trivially_constructible_v<non_trivial_vector>, "non_trivial_vector is trivially default constructible.");

static_assert(std::is_trivially_copy_constructible_v<trivial_vector>, "trivial_vector isn't trivially copy constructible.");
static_assert(!std::is_trivially_copy_constructible_v<non_trivial_vector>, "non_trivial_vector is trivially copy constructible.");
static_assert(std::is_trivially_copy_assignable_v<trivial_vector>, "trivial_vector isn't trivially copy assignable_v.");
static_assert(!std::is_trivially_copy_assignable_v<non_trivial_vector>, "non_trivial_vector is trivially copy assignable.");

static_assert(std::is_trivially_move_constructible_v<trivial_vector>, "trivial_vector isn't trivially move constructible.");
static_assert(!std::is_trivially_move_constructible_v<non_trivial_vector>, "non_trivial_vector is trivially move constructible.");
static_assert(std::is_trivially_move_assignable_v<trivial_vector>, "trivial_vector isn't trivially move assignable.");
static_assert(!std::is_trivially_move_assignable_v<non_trivial_vector>, "non_trivial_vector is trivially move assignable.");

static_assert(std::is_trivially_destructible_v<trivial_vector>, "trivial_vector isn't trivially destructible.");
static_assert(!std::is_trivially_destructible_v<non_trivial_vector>, "non_trivial_vector is trivially destructible.");

static_assert(std::is_trivially_copyable_v<trivial_vector>, "trivial_vector isn't trivially copyable.");
static_assert(!std::is_trivially_copyable_v<non_trivial_vector>, "non_trivial_vector is trivially copyable.");

static_assert(std::is_standard_layout_v<trivial_vector>, "trivial_vector isn't standard layout.");
static_assert(std::is_standard_layout_v<non_trivial_vector>, "trivial_vector isn't standard layout.");

int main()
{
}