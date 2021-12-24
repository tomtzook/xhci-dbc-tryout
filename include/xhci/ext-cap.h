#pragma once

#include <definition.h>


#define XHCI_HCCPARAMS1_OFFSET (0x10)

typedef struct {
    uint32_t ignored : 16;
    volatile uint32_t xecp : 16;
} xhci_hccparams1_register_t;

typedef enum {
    XHCI_EXTCAP_USB_LEGACY = 1,
    XHCI_EXTCAP_SUPPORTED_PROTOCOL = 2,
    XHCI_EXTCAP_EXTENDED_POWER_MANAGEMENT = 3,
    XHCI_EXTCAP_IO_VIRTUALIZATION = 4,
    XHCI_EXTCAP_MESSAGE_INTERRUPT = 5,
    XHCI_EXTCAP_LOCAL_MEMORY = 6,
    XHCI_EXTCAP_USB_DBC = 10,
} xhci_extcap_id_t;

typedef struct {
    volatile uint32_t capability_id : 8;
    volatile uint32_t next_extcap_ptr : 8;
} xhci_extcap_register_base_t;


int xhci_extcap_find_next(const volatile void* mmio_base, uint32_t id, size_t* offset);
