#pragma once

#include <stdint.h> // for some types
#include <stddef.h> // for some macros
#include <string.h> // for memory
#include <sys/io.h> // for port access

// write fence
#define wmb()
// read fence
#define rmb()

#define BIT(x) (1ULL << (x))

#define PAGE_SHIFT		12
#define PAGE_SIZE		(1UL << PAGE_SHIFT)
