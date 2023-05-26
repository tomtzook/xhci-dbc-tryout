#include "error_handling.h"
#include "hw/pci.h"
#include "xhci/ext-cap.h"
#include "utils.h"
#include "xdbc/xdbc.h"

// https://elixir.bootlin.com/linux/latest/source/drivers/usb/early/xhci-dbc.h#L74
// https://elixir.bootlin.com/linux/latest/source/drivers/usb/early/xhci-dbc.c

// allocate a single page for the data table. should be enough for now
#define DATA_TABLE_PAGE_COUNT (1)


static int map_xhci_mmio(uint32_t bus, uint32_t device, uint32_t func, volatile void** mmio_base, size_t* mmio_size) {
    // check if the device can work with mmio space. If not, enable it to do so.
    pci_command_t command;
    command.data = pci_read_config16(bus, device, func, PCI_COMMAND);
    if (!command.bits.mem_space) {
        command.bits.mem_space = 1;
        pci_write_config16(bus, device, func, PCI_COMMAND, command.data);
    }

    // the mmio base address is in BAR0 register of PCI config space.
    // the amount of space can be retrieved by setting the BAR to all 1s
    // and then read again

    pci_bar_t bar;
    pci_bar_t bar_size;

    bar.data = pci_read_config32(bus, device, func, PCI_BASE_ADDRESS_0);
    pci_write_config32(bus, device, func, PCI_BASE_ADDRESS_0, ~0);
    bar_size.data = pci_read_config32(bus, device, func, PCI_BASE_ADDRESS_0);
    pci_write_config32(bus, device, func, PCI_BASE_ADDRESS_0, bar.data);

    uint64_t address = bar.bits.address << PCI_BAR_MEM_ADDRESS_SHIFT;
    uint64_t size = bar_size.bits.address << PCI_BAR_MEM_ADDRESS_SHIFT;

    // if address is 64bit, we need to read BAR1 as well which will
    // contain the higher 32 bits of the address
    if (PCI_BAR_64_ADDRESS == bar.bits.type) {
        bar.data = pci_read_config32(bus, device, func, PCI_BASE_ADDRESS_0 + 4);
        pci_write_config32(bus, device, func, PCI_BASE_ADDRESS_0 + 4, ~0);
        bar_size.data = pci_read_config32(bus, device, func, PCI_BASE_ADDRESS_0 + 4);
        pci_write_config32(bus, device, func, PCI_BASE_ADDRESS_0 + 4, bar.data);

        address |= ((uint64_t)bar.bits.address << PCI_BAR_MEM_ADDRESS_SHIFT);
        size |= ((uint64_t)bar_size.bits.address << PCI_BAR_MEM_ADDRESS_SHIFT);
    }

    // find virtual address. remove caching for it.
    // what else?
    *mmio_base = (void*) address;
    *mmio_size = size;

    return ERROR_SUCCESS;
}

static void ring_doorbell(xdbc_context* context, xhci_dbc_doorbell_t doorbell) {
    context->dbc_register->dcdb.db_target = doorbell;
}

int xdbc_init(xdbc_context* context) {
    int status = ERROR_SUCCESS;

    memset(context, 0, sizeof(xdbc_context));

    // find controller bus
    // iterate over the pci bus devices until the xhc controller is found
    GOTO_CLEAN_ON_ERROR(pci_find_class(PCI_CLASS_SERIAL_USB_XHCI, &context->bus, &context->dev, &context->func));

    // map mmio regs
    // read info from pci bus regarding the location of mmio
    volatile void* mmio_base = NULL;
    size_t mmio_size;
    GOTO_CLEAN_ON_ERROR(map_xhci_mmio(context->bus, context->dev, context->func, &mmio_base, &mmio_size));

    // find extcap regs
    size_t regs_offset;
    GOTO_CLEAN_ON_ERROR(xhci_extcap_find_next(mmio_base, XHCI_EXTCAP_USB_DBC, &regs_offset));
    context->dbc_register = (volatile xhci_dbc_register_t*) (mmio_base + regs_offset);

    // allocate memory for contexts and rings
    context->data_table = env_allocate_pages(DATA_TABLE_PAGE_COUNT);
    GOTO_CLEAN_ON_ERROR(NULL == context->data_table ? ERROR_MEMORY_ALLOCATION : ERROR_SUCCESS);

    GOTO_CLEAN_ON_ERROR(xhci_ring_alloc(&context->evt_ring, 1));
    GOTO_CLEAN_ON_ERROR(xhci_ring_alloc(&context->out_ring, 1));
    GOTO_CLEAN_ON_ERROR(xhci_ring_alloc(&context->in_ring, 1));

    // enable dbc?
    wmb();
    context->dbc_register->dcctrl.bits.dce = 1;
    wmb();
    GOTO_CLEAN_ON_ERROR(wait_for_set(&context->dbc_register->dcctrl, DBC_DCCTRL_DCE_BIT, 1));

    wmb();
    context->dbc_register->dcportsc.ped = 1;
    wmb();

    GOTO_CLEAN_ON_ERROR(wait_for_set(&context->dbc_register->dcctrl, DBC_DCCTRL_DCR_BIT, 1));

    // fill data table with context information

    // reset structures
    xhci_ring_reset(&context->evt_ring);
    xhci_ring_reset(&context->out_ring);
    xhci_ring_reset(&context->in_ring);

    // start dbc

clean:
    env_free_pages(context->data_table, DATA_TABLE_PAGE_COUNT);
    xhci_ring_free(&context->evt_ring);
    xhci_ring_free(&context->out_ring);
    xhci_ring_free(&context->in_ring);

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
