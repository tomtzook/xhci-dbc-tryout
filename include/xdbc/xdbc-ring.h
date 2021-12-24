#pragma once

#include <definition.h>
#include <xhci/ring.h>


// we'll use a single-segment, static-sized ring
// no support for dynamic extending and such for now
typedef struct {
    xhci_ring_t ring;
    void* segment_base;
    size_t segment_pages_count;
} xdbc_ring_t;


int xdbc_ring_alloc(xdbc_ring_t* ring, size_t pages_count);
void xdbc_ring_free(xdbc_ring_t* ring);
void xdbc_ring_reset(xdbc_ring_t* ring);
