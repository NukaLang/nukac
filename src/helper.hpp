#ifndef NUKAC_HELPER_HPP
#define NUKAC_HELPER_HPP

#include <variant>

#include <cstdint>
#include <string>
#include <sys/types.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using usize = size_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using size = ssize_t;

namespace nukac::helper {
  void exceptionHandler(std::string msg);

  using Value = std::variant <
    std::string,
    usize,
    size,
    long double
  >;
} // nukac::helper
 
#endif // NUKAC_HELPER_HPP
