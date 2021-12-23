
#include "include/xhci/ring.h"


int main() {
    char ring_buffer[1024] = {0};
    xhci_ring_t ring;
    ring.enqueue.ptr = (xhci_trb_t*) ring_buffer;
    ring.dequeue.ptr = (xhci_trb_t*) ring_buffer;
    ring.cycle_bit = 1;

    // enqueue trb
    char buffer[5];
    xhci_trb_t trb = {0};
    trb.parameter.pointer = buffer;
    trb.status.length = sizeof(buffer);
    trb.control.trb_type = TRB_TYPE_NORMAL;
    trb.control.interrupt_on_completion = 1;
    xhci_ring_enqueue(&ring, &trb);

    // dequeue trb
    xhci_trb_t trb2 = {0};
    xhci_ring_dequeue(&ring, &trb2);

    return 0;
}
