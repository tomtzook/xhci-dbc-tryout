#pragma once

#include "definitions.h"


int wait_for_set(volatile void* ptr, uint32_t mask, uint32_t done);
