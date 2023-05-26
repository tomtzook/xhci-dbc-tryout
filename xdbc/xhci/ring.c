
#include "hw/environment.h"
#include "error_handling.h"
#include "xhci/ring.h"


int xhci_ring_advance(xhci_ring_t* ring, xhci_ring_ptr_t* ptr) {
    ++(ptr->ptr);

    if (TRB_TYPE_LINK == ptr->ptr->control.trb_type) {
        ptr->ptr->control.cycle_bit = ring->cycle_bit;
        ptr->ptr = (xhci_trb_t*) ptr->ptr->parameter.pointer;
        ring->cycle_bit ^= 1;
    }

    return 0;
}

int xhci_ring_enqueue(xhci_ring_t* ring, xhci_trb_t* trb) {
    xhci_ring_ptr_t* enqueue = &ring->enqueue;
    memcpy(enqueue->ptr, trb, sizeof(xhci_trb_t));

    wmb();
    enqueue->ptr->control.cycle_bit = ring->cycle_bit;

    return xhci_ring_advance(ring, enqueue);
}

int xhci_ring_dequeue(xhci_ring_t* ring, xhci_trb_t* trb) {
    xhci_ring_ptr_t* dequeue = &ring->dequeue;
    if (ring->cycle_bit != dequeue->ptr->control.cycle_bit) {
        return 1;
    }

    rmb();
    memcpy(trb, dequeue->ptr, sizeof(xhci_trb_t));
    return xhci_ring_advance(ring, dequeue);
}

int xhci_ring_alloc(xhci_producer_ring_t* ring, size_t pages_count) {
    void* base = env_allocate_pages(pages_count);
    if (NULL == base) {
        return ERROR_MEMORY_ALLOCATION;
    }

    ring->segment_base = base;
    ring->segment_pages_count = pages_count;

    return 0;
}

void xhci_ring_free(xhci_producer_ring_t* ring) {
    if (NULL == ring->segment_base) {
        return;
    }

    env_free_pages(ring->segment_base, ring->segment_pages_count);
}

void xhci_ring_reset(xhci_producer_ring_t* ring) {
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
