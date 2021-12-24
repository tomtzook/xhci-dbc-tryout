#include <environment.h>
#include <error_handling.h>
#include <xdbc/xdbc-ring.h>


int xdbc_ring_alloc(xdbc_ring_t* ring, size_t pages_count) {
    void* base = env_allocate_pages(pages_count);
    if (NULL == base) {
        return ERROR_MEMORY_ALLOCATION;
    }

    ring->segment_base = base;
    ring->segment_pages_count = pages_count;

    return 0;
}

void xdbc_ring_free(xdbc_ring_t* ring) {
    if (NULL == ring->segment_base) {
        return;
    }

    env_free_pages(ring->segment_base, ring->segment_pages_count);
}

void xdbc_ring_reset(xdbc_ring_t* ring) {
    memset(ring->segment_base, 0, PAGE_SIZE * ring->segment_pages_count);

    ring->ring.enqueue.ptr = (xhci_trb_t*) ring->segment_base;
    ring->ring.dequeue.ptr = (xhci_trb_t*) ring->segment_base;
    ring->ring.cycle_bit = 1;

    // setup the last as a link trb cycling to the first
    xhci_trb_t* trbs = (xhci_trb_t*) ring->segment_base;
    xhci_trb_t* last_trb = &trbs[XHCI_TRBS_PER_PAGE * ring->segment_pages_count - 1];
    last_trb->parameter.data = env_virtual_to_physical(ring->segment_base);
    last_trb->control.trb_type = TRB_TYPE_LINK;
    last_trb->control.evaluate_next = 1;
}
