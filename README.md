# `dpm::static_vector`

An implementation of [P0843][] using ++20. Example usage:

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


[P0843]: https://wg21.link/P0843