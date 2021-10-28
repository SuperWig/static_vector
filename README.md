# `dpm::static_vector`

An implementation of [P0843][] using C++20. Example usage:

```cpp
#include <dpm/static_vector.h>

int main()
{
    dpm::static_vector<int, 5> vec{ 1, 2 };
    vec.push_back(3);
    vec.pop();
    return vec.size() - 2;
}
```
## To Build / Install

```
cmake -B build .
cmake --build build
# Or to build & install
sudo cmake --build build -t install
```
The CMake target is `dpm::static_vector` and the `find_package` is `dpm-static_vector`.

---

Note: The `constexpr`-ness is basically a lie due to the use of `reinterpret_cast` and the fact that the `unitialized_` algorithms aren't `constexpr` ([P2283][] proposes making them so).  
It should be possible for it to be constexpr for trivial types by using `std::array` if the types are trivial and by not using the algorithms. 


[P0843]: https://wg21.link/P0843
[P2283]: https://wg21.link/P2283