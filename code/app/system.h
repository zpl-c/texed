#pragma once

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>

#define ZPL_NANO
#include "zpl.h"

#define defer_var ZPL_CONCAT(_i_,__LINE__)
#define defer(s,e) for ( \
uint32_t defer_var = (s, 0); \
!defer_var; \
(defer_var += 1), e) \
