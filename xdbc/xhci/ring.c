
#include "hw/environment.h"
#include "error_handling.h"
#include "xhci/ring.h"


void xhci_ring_reset(xhci_ring_t* ring) {
    memset(ring->base, 0, ring->size);

    ring->enqueue = (xhci_trb_t*) ring->base;
    ring->dequeue = (xhci_trb_t*) ring->base;
    ring->cycle_bit = 1;

    // setup the last as a link trb cycling to the first
    xhci_trb_t* trbs = (xhci_trb_t*) ring->base;
    xhci_trb_t* last_trb = &trbs[XHCI_TRBS_PER_PAGE * (ring->size >> PAGE_SHIFT) - 1];
    last_trb->parameter.data = env_virtual_to_physical(ring->base);
    last_trb->control.trb_type = TRB_TYPE_LINK;
    last_trb->control.evaluate_next = 1;
}

void xhci_ring_advance(xhci_ring_t* ring, xhci_trb_t** ptr) {
    ++(*ptr);

    if (TRB_TYPE_LINK == (*ptr)->control.trb_type) {
        (*ptr)->control.cycle_bit = ring->cycle_bit;
        *ptr = (xhci_trb_t*) (*ptr)->parameter.pointer;
        ring->cycle_bit ^= 1;
    }
}

void xhci_ring_enqueue(xhci_ring_t* ring, xhci_trb_t* trb) {
    xhci_trb_t* enqueue = ring->enqueue;
    memcpy(enqueue, trb, sizeof(xhci_trb_t));

    wmb();
    enqueue->control.cycle_bit = ring->cycle_bit;
    xhci_ring_advance(ring, &enqueue);
}

void xhci_ring_dequeue(xhci_ring_t* ring, xhci_trb_t* trb) {
    xhci_trb_t* dequeue = ring->dequeue;
    if (ring->cycle_bit != dequeue->control.cycle_bit) {
        return 1;
    }

    rmb();
    memcpy(trb, dequeue, sizeof(xhci_trb_t));
    xhci_ring_advance(ring, &dequeue);
}
