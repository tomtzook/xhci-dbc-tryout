#pragma once

#include <definition.h>

#pragma pack(push, 1)

typedef struct {
    union {
        uint64_t data;
        void* pointer;
    } parameter;
    union {
        struct {
            uint32_t length : 17;
            uint32_t td_size : 5;
            uint32_t interrupter_target : 10;
        };
        uint32_t all;
    } status;
    union {
        struct {
            uint32_t cycle_bit : 1;
            uint32_t evaluate_next : 1;
            uint32_t interrupt_on_short_package : 1;
            uint32_t no_snoop : 1;
            uint32_t chain_bit : 1;
            uint32_t interrupt_on_completion : 1;
            uint32_t immediate_data : 1;
            uint32_t reserved0 : 2;
            uint32_t block_event_interrupt : 1;
            uint32_t trb_type : 6;
            uint32_t reserved1 : 16;
        };
        uint32_t all;
    } control;
} xhci_trb_t;

//P.470
/*
struct trb_control_setup_t {
    union {
        struct {
            uint64_t bmRequestType : 8;
            uint64_t bRequest : 8;
            uint64_t wValue : 16;
            uint64_t wIndex : 16;
            uint64_t wLength : 16;
        };
        uint64_t all;
    } parameter;
    union {
        struct {
            uint32_t length : 17;
            uint32_t reserved0 : 5;
            uint32_t interrupter_target : 10;
        };
        uint32_t all;
    } status;
    union {
        struct {
            uint32_t cycle_bit : 1;
            uint32_t reserved : 4;
            uint32_t interrupt_on_completion : 1;
            uint32_t immediate_data : 1;
            uint32_t reserved0 : 3;
            uint32_t trb_type : 6;
            uint32_t transfer_type : 2;
            uint32_t reserved1 : 14;
        };
        uint32_t all;
    } control;
};
*/

#pragma pack(pop)

typedef enum {
    TRB_TYPE_NORMAL = 1,
    TRB_TYPE_LINK = 6,
} xhci_trb_type_t;

typedef struct {
    xhci_trb_t* ptr;
} xhci_ring_ptr_t;

typedef struct {
    xhci_ring_ptr_t enqueue;
    xhci_ring_ptr_t dequeue;

    uint32_t cycle_bit;
} xhci_ring_t;

int xhci_ring_advance(xhci_ring_t* ring, xhci_ring_ptr_t* ptr);

int xhci_ring_enqueue(xhci_ring_t* ring, xhci_trb_t* trb);
int xhci_ring_dequeue(xhci_ring_t* ring, xhci_trb_t* trb);
