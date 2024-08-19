#ifndef SRC_TYPES_HPP_
#define SRC_TYPES_HPP_

#include <string>

using std::size_t;
using std::string;
using namespace std::string_literals;

#define HashMap std::unordered_map
#define HashSet std::unordered_set

#define foreach_key(i, map)                                                    \
  for ([[maybe_unused]] const auto &[i, _discard##__LINE__] : map)
#define foreach_cvalue(i, map)                                                 \
  for ([[maybe_unused]] const auto &[_discard##__LINE__, i] : map)
#define foreach_mvalue(i, map)                                                 \
  for ([[maybe_unused]] auto &[_discard##__LINE__, i] : map)

typedef signed char sbyte;
typedef unsigned char ubyte;

#ifndef __linux__
typedef unsigned short ushort;
typedef unsigned uint;
typedef unsigned long ulong;
#endif

typedef long long int64;
typedef unsigned long long uint64;

#endif /* SRC_TYPES_HPP_ */
