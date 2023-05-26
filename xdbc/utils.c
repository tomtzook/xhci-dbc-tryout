
#include "definitions.h"
#include "hw/environment.h"
#include "utils.h"


int wait_for_set(volatile void* ptr, uint32_t mask, uint32_t done) {
    // wait until a set of bits are set to a specific value
    size_t wait_time_usec = 1000;
    size_t delay_time_usec = 10;

    do {
        uint32_t value = *(volatile uint32_t*)ptr;
        if ((value & mask) == done) {
            return ERROR_SUCCESS;
        }

        env_sleep_usec(delay_time_usec);
        wait_time_usec -= delay_time_usec;
    } while (wait_time_usec > 0);

    return ERROR_TIMEOUT;
}
