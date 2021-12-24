#pragma once

#include <definition.h>


void* env_allocate_pages(size_t amount);
void env_free_pages(void* base, size_t amount);

uint64_t env_virtual_to_physical(void* address);

void env_sleep_usec(size_t usec);
