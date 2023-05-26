#include "error_handling.h"
#include "hw/pci.h"
#include "xhci/ext-cap.h"
#include "utils.h"
#include "xdbc/xdbc.h"

// https://elixir.bootlin.com/linux/latest/source/drivers/usb/early/xhci-dbc.h#L74
// https://elixir.bootlin.com/linux/latest/source/drivers/usb/early/xhci-dbc.c

// allocate a single page for the data table. should be enough for now
#define DATA_TABLE_PAGE_COUNT sizeof(xdbc_data_table) >> PAGE_SHIFT

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

static int allocate_data(xdbc_context* context) {
    int status = ERROR_SUCCESS;

    // allocate memory for contexts and rings
    context->data_table = env_allocate_pages(DATA_TABLE_PAGE_COUNT);
    GOTO_CLEAN_ON_ERROR(NULL == context->data_table ? ERROR_MEMORY_ALLOCATION : ERROR_SUCCESS);

    context->evt_ring.base = context->data_table->event_ring_data;
    context->evt_ring.size = sizeof(context->data_table->event_ring_data);
    context->in_ring.base = context->data_table->in_ring_data;
    context->in_ring.size = sizeof(context->data_table->in_ring_data);
    context->out_ring.base = context->data_table->out_ring_data;
    context->out_ring.size = sizeof(context->data_table->out_ring_data);

clean:
    env_free_pages(context->data_table, DATA_TABLE_PAGE_COUNT);

    return status;
}

static int find_dbc_registers(xdbc_context* context) {
    int status = ERROR_SUCCESS;

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

clean:
    return status;
}

static void xhci_init_endpoints(xdbc_context* context) {
    xhci_dbc_endpoint_context_t* ep_reg;

    ep_reg = &context->data_table->dbc_ctx.out;
    ep_reg->info.max_packet_size = 1024;
    ep_reg->info.max_burst_size = context->dbc_register->dcctrl.bits.debug_max_burst_size;
    ep_reg->info.ep_type = DBC_EP_BULK_OUT;
    ep_reg->dequeue.tr_dequeue_ptr = env_virtual_to_physical(context->out_ring.base);

    ep_reg = &context->data_table->dbc_ctx.in;
    ep_reg->info.max_packet_size = 1024;
    ep_reg->info.max_burst_size = context->dbc_register->dcctrl.bits.debug_max_burst_size;
    ep_reg->info.ep_type = DBC_EP_BULK_IN;
    ep_reg->dequeue.tr_dequeue_ptr = env_virtual_to_physical(context->in_ring.base);
}

static void xhci_init_info(xdbc_context* context) {
    const char strings[] = {
            6,  3, 9, 0, 4, 0,
            8,  3, 'A', 0, 'I', 0, 'S', 0,
            30, 3, 'X', 0, 'D', 0, 'B', 0, 'C', 0,
            'D', 0, 'B', 0, 'C', 0, ' ', 0,
            'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0,
            4, 3, '0', 0
    };

    memcpy(context->data_table->info_strs, strings, sizeof(strings));

    uint64_t strings_base = env_virtual_to_physical(context->data_table->info_strs);
    xhci_dbc_context_t* dbc_ctx = &context->data_table->dbc_ctx;

    dbc_ctx->info.string0.address = strings_base;
    dbc_ctx->info.length.string0 = 6;
    dbc_ctx->info.manufacturer_string.address = strings_base + 6;
    dbc_ctx->info.length.manufacturer_string = 8;
    dbc_ctx->info.product_string.address = strings_base + 6 + 8;
    dbc_ctx->info.length.product_string = 30;
    dbc_ctx->info.serial_number_string.address = strings_base + 6 + 8 + 30;
    dbc_ctx->info.length.serial_number_string = 4;
}



static void ring_doorbell(xdbc_context* context, xhci_dbc_doorbell_t doorbell) {
    context->dbc_register->dcdb.db_target = doorbell;
}

int xdbc_init(xdbc_context* context) {
    int status = ERROR_SUCCESS;

    memset(context, 0, sizeof(xdbc_context));
    GOTO_CLEAN_ON_ERROR(allocate_data(context));
    GOTO_CLEAN_ON_ERROR(find_dbc_registers(context));

    // reset and configure rings
    context->evt_ring.base = context->data_table->event_ring_data;
    context->evt_ring.size = sizeof(context->data_table->event_ring_data);
    xhci_ring_reset(&context->evt_ring);

    context->out_ring.base = context->data_table->out_ring_data;
    context->out_ring.size = sizeof(context->data_table->out_ring_data);
    xhci_ring_reset(&context->out_ring);

    context->in_ring.base = context->data_table->in_ring_data;
    context->in_ring.size = sizeof(context->data_table->in_ring_data);
    xhci_ring_reset(&context->in_ring);

    // fill data table with context information and fill registers
    xhci_init_info(context);
    xhci_init_endpoints(context);

    context->dbc_register->dccp.context_ptr = env_virtual_to_physical(&context->data_table->dbc_ctx);

    // set event segment
    context->data_table->erst_segment.base = env_virtual_to_physical(context->data_table->event_ring_data);
    context->data_table->erst_segment.size = sizeof(context->data_table->event_ring_data);
    context->dbc_register->dcerstba.erst_base_address = env_virtual_to_physical(&context->data_table->erst_segment);
    context->dbc_register->dcerstsz.erst_size = 1;

    // enable dbc
    xhci_enable_dbc(context->dbc_register);

    // start dbc

clean:

    return status;
}

int xdbc_request_write(xdbc_context* context, const void* buffer, size_t size) {
    xhci_trb_t trb = {0};
    trb.parameter.pointer = (void*) buffer; // not actually modified though
    trb.status.length = size;
    trb.control.trb_type = TRB_TYPE_NORMAL;
    trb.control.interrupt_on_completion = 1;

    xhci_ring_enqueue(&context->out_ring, &trb);

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

    xhci_ring_enqueue(&context->in_ring, &trb);

    // linux's xhci-dbc touches the cycle bit of the enqueued trb here
    // why? we already update it earlier....

    ring_doorbell(context, DBC_DOORBELL_EP_IN);
    return 0;
}
