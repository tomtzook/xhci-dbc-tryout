#pragma once

#include <definition.h>

// defines a set of functions which the platform needs to provide
// by replacing the implementation, different platforms can use this
// library

// memory
void* env_allocate_pages(size_t amount);
void env_free_pages(void* base, size_t amount);
uint64_t env_virtual_to_physical(void* address);

// time
void env_sleep_usec(size_t usec);
