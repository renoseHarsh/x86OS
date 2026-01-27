#pragma once

#define IS_ALIGNED(x, align) ((x & (align - 1)) == 0)
#define ALIGN_UP(x, align) ((x + align - 1) & ~(align - 1))
