#include <xhci/ring.h>


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
