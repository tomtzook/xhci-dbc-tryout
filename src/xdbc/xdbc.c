#include <error_handling.h>
#include <xhci/ext-cap.h>
#include <xdbc/xdbc.h>

// https://elixir.bootlin.com/linux/latest/source/drivers/usb/early/xhci-dbc.h#L74
// https://elixir.bootlin.com/linux/latest/source/drivers/usb/early/xhci-dbc.c

// allocate a single page for the data table. should be enough for now
#define DATA_TABLE_PAGE_COUNT (1)

static int wait_for_set(volatile void* ptr, uint32_t mask, uint32_t done, size_t wait_time_usec, size_t delay_time_usec) {
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

static void ring_doorbell(xdbc_context* context, xhci_dbc_doorbell_t doorbell) {
    context->dbc_register->dcdb.db_target = doorbell;
}

int xdbc_init(xdbc_context* context) {
    int status = ERROR_SUCCESS;

    memset(context, 0, sizeof(xdbc_context));

    // find controller bus
    // iterate over the pci bus devices until the xhc controller is found

    // map mmio regs
    // read info from pci bus regarding the location of mmio
    volatile void* mmio_base = NULL;

    // find extcap regs
    size_t regs_offset;
    GOTO_CLEAN_ON_ERROR(xhci_extcap_find_next(mmio_base, XHCI_EXTCAP_USB_DBC, &regs_offset));
    context->dbc_register = (volatile xhci_dbc_register_t*) (mmio_base + regs_offset);

    // enable dbc?

    context->data_table = env_allocate_pages(DATA_TABLE_PAGE_COUNT);
    GOTO_CLEAN_ON_ERROR(NULL == context->data_table ? ERROR_MEMORY_ALLOCATION : ERROR_SUCCESS);

    GOTO_CLEAN_ON_ERROR(xdbc_ring_alloc(&context->evt_ring, 1));
    GOTO_CLEAN_ON_ERROR(xdbc_ring_alloc(&context->out_ring, 1));
    GOTO_CLEAN_ON_ERROR(xdbc_ring_alloc(&context->in_ring, 1));

clean:
    env_free_pages(context->data_table, DATA_TABLE_PAGE_COUNT);
    xdbc_ring_free(&context->evt_ring);
    xdbc_ring_free(&context->out_ring);
    xdbc_ring_free(&context->in_ring);

    return status;
}

int xdbc_request_write(xdbc_context* context, const void* buffer, size_t size) {
    xhci_trb_t trb = {0};
    trb.parameter.pointer = (void*) buffer; // not actually modified though
    trb.status.length = size;
    trb.control.trb_type = TRB_TYPE_NORMAL;
    trb.control.interrupt_on_completion = 1;

    RETURN_ON_ERROR(xhci_ring_enqueue(&context->out_ring.ring, &trb));

    // linux's xhci-dbc touches the cycle bit of the enqueued trb here
    // why? we already update it earlier....

    ring_doorbell(context, DBC_DOORBELL_EP_OUT);
    return 0;
}

int xdbc_request_read(xdbc_context* context, void* buffer, size_t size) {
    xhci_trb_t trb = {0};
    trb.parameter.pointer = buffer;
    trb.status.length = size;
    trb.control.trb_type = TRB_TYPE_NORMAL;
    trb.control.interrupt_on_completion = 1;

    RETURN_ON_ERROR(xhci_ring_enqueue(&context->in_ring.ring, &trb));

    // linux's xhci-dbc touches the cycle bit of the enqueued trb here
    // why? we already update it earlier....

    ring_doorbell(context, DBC_DOORBELL_EP_IN);
    return 0;
}
