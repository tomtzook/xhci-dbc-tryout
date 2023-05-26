#pragma once

#include "types.h"


static inline int memcmp(const void* s1, const void* s2, size_t size) {
    uint8_t* ptr_s1 = (uint8_t*) s1;
    uint8_t* ptr_s2 = (uint8_t*) s2;
    while (size-- > 0) {
        if (*ptr_s1++ != *ptr_s2++)
            return ptr_s1[-1] < ptr_s2[-1] ? -1 : 1;
    }
    return 0;
}

static inline void memset(void* dest, uint8_t value, size_t size) {
    uint8_t* ptr = (uint8_t*) dest;
    while ((size--)) {
        *ptr = value;
        ++(ptr);
    }
}

static inline void* memcpy(void* dest, const void* src, size_t size) {
    uint8_t* ptr_src = (uint8_t*) src;
    uint8_t* ptr_dest = (uint8_t*) dest;
    while ((size--)) {
        *ptr_dest = *ptr_src;
        ++(ptr_src);
        ++(ptr_dest);
    }

    return dest;
}

static inline size_t strlen(const char* s) {
    size_t count = 0;
    while (*s) {
        ++count;
        ++(s);
    }

    return count;
}

static inline int strcmp(const char* s1, const char* s2) {
    while (*s1) {
        if (*s1 != *s2) {
            break;
        }
        ++(s1);
        ++(s2);
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}
