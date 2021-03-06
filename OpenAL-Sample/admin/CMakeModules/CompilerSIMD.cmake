MACRO(CHECK_MMX)
IF(CMAKE_COMPILER_IS_GNUCC)
  SET(MMX_FLAGS "-mmmx")

  SET(CMAKE_REQUIRED_FLAGS "${MMX_FLAGS}")
  CHECK_C_SOURCE_COMPILES(
"int main(){__builtin_ia32_emms();}
" HAVE_MMX)
SET(CMAKE_REQUIRED_FLAGS "")
ELSE(CMAKE_COMPILER_IS_GNUCC)

  CHECK_C_SOURCE_COMPILES(
"#include <mmintrin.h>
int main() {_mm_empty();}
" HAVE_MMX)
ENDIF(CMAKE_COMPILER_IS_GNUCC)
ENDMACRO(CHECK_MMX)


MACRO(CHECK_SSE2)
IF(CMAKE_COMPILER_IS_GNUCC)
  SET(SSE2_FLAGS "-mmmx -msse -msse2")

  SET(CMAKE_REQUIRED_FLAGS "${SSE2_FLAGS}")
  CHECK_C_SOURCE_COMPILES(
"int main() {__builtin_ia32_loadupd(0);}
" HAVE_SSE2)
  SET(CMAKE_REQUIRED_FLAGS "")
ELSE(CMAKE_COMPILER_IS_GNUCC)

  CHECK_C_SOURCE_COMPILES(
"#include <emmintrin.h>
int main() {_mm_loadu_pd(0);}
" HAVE_SSE2)
ENDIF(CMAKE_COMPILER_IS_GNUCC)
ENDMACRO(CHECK_SSE2)
