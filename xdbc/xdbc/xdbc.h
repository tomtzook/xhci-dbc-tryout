#pragma once

#include "definitions.h"
#include "xhci/dbc.h"
#include "xhci/ring.h"


typedef struct {
    xhci_dbc_context_t dbc_ctx;
    xue_erst_segment erst_segment;

    char info_strs[PAGE_SIZE];

    char event_ring_data[PAGE_SIZE];
    char in_ring_data[PAGE_SIZE];
    char out_ring_data[PAGE_SIZE];
} xdbc_data_table;

typedef struct {
    uint32_t bus;
    uint32_t dev;
    uint32_t func;

    volatile xhci_dbc_register_t* dbc_register;

    xdbc_data_table* data_table;

    xhci_ring_t evt_ring;
    xhci_ring_t out_ring;
    xhci_ring_t in_ring;
} xdbc_context;


int xdbc_init(xdbc_context* context);

// enqueues work items for writing/reading.
// this is an async execution
int xdbc_request_write(xdbc_context* context, const void* buffer, size_t size);
int xdbc_request_read(xdbc_context* context, void* buffer, size_t size);
