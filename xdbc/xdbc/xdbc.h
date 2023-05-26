#pragma once

#include "definitions.h"
#include "xhci/dbc.h"
#include "xhci/ring.h"


typedef struct {

} xdbc_data_table;

typedef struct {
    uint32_t bus;
    uint32_t dev;
    uint32_t func;

    volatile xhci_dbc_register_t* dbc_register;

    xdbc_data_table* data_table;

    xhci_dbc_info_context_t info;
    xhci_dbc_endpoint_context_t out;
    xhci_dbc_endpoint_context_t in;

    xhci_producer_ring_t evt_ring;
    xhci_producer_ring_t out_ring;
    xhci_producer_ring_t in_ring;
} xdbc_context;


int xdbc_init(xdbc_context* context);

// enqueues work items for writing/reading.
// this is an async execution
int xdbc_request_write(xdbc_context* context, const void* buffer, size_t size);
int xdbc_request_read(xdbc_context* context, void* buffer, size_t size);
