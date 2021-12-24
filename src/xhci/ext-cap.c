#include <error_handling.h>
#include <xhci/ext-cap.h>


int xhci_find_next_ext_cap(const volatile void* mmio_base, uint32_t id, size_t* offset) {
    const volatile xhci_hccparams1_register_t* reg =
            (const volatile xhci_hccparams1_register_t*) (mmio_base + XHCI_HCCPARAMS1_OFFSET);
    uint32_t reg_offset = reg->xecp << 2;
    if (!reg_offset) {
        return 1; // error: no caps
    }

    uint32_t next;
    do {
        const volatile xhci_extcap_register_base_t* extcap_reg =
                (const volatile xhci_extcap_register_base_t*) (mmio_base + reg_offset);
        if (extcap_reg->capability_id == id) {
            *offset = reg_offset;
            return ERROR_SUCCESS;
        }

        next = extcap_reg->next_extcap_ptr;
        reg_offset += next << 2;
    } while (next);

    return 1; // error: not found
}
