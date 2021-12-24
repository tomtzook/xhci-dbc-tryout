#pragma once

typedef enum {
    ERROR_SUCCESS = 0,
    ERROR_MEMORY_ALLOCATION = 1,
    ERROR_TIMEOUT = 2,
    ERROR_NOT_FOUND = 3,
} error_code_t;


#define RETURN_ON_ERROR(...) \
    do {                     \
        int _res = __VA_ARGS__; \
        if (_res) {           \
            return _res;                     \
        }                         \
    } while(0)

#define GOTO_CLEAN_ON_ERROR(...) \
    do {                     \
        int _res = __VA_ARGS__; \
        if (_res) {           \
            status = _res;       \
            goto clean;         \
        }                         \
    } while(0)
