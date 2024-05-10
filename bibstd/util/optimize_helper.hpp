#pragma once

#if defined(__clang__)
  #define CLANG_COMPILER
#elif defined(__GNUC__) || defined(__GNUG__)
  #define GCC_COMPILER
#endif

#if defined(GCC_COMPILER)
  #define NO_OPTIMIZATION __attribute__((optimize("O0")))
#elif defined(CLANG_COMPILER)
  #define NO_OPTIMIZATION __attribute__((optnone))
#endif
