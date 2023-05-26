#pragma once

#include "types.h"
#include "hw/io.h"
#include "intrinsics.h"
#include "error_handling.h"

// write fence
#define wmb()
// read fence
#define rmb()

#define BIT(x) (1ULL << (x))

#define PAGE_SHIFT		12
#define PAGE_SIZE		(1UL << PAGE_SHIFT)
#define NULL            ((void*)0)
