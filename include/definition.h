#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

// write fence
#define wmb()
// read fence
#define rmb()

#define BIT(x) (1ULL << (x))

#define PAGE_SHIFT		12
#define PAGE_SIZE		(1UL << PAGE_SHIFT)
