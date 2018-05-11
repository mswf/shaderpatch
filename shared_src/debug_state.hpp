#pragma once

namespace sp {

#ifdef NDEBUG
inline constexpr bool debug_build = false;
#else
inline constexpr bool debug_build = true;
#endif

}
