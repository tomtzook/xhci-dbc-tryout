#pragma once

#include <definition.h>
#include <xhci/dbc.h>
#include "xdbc-ring.h"


typedef struct {

} xdbc_data_table;

typedef struct {
    volatile xhci_dbc_register_t* dbc_register;

    xdbc_data_table* data_table;

    xdbc_ring_t evt_ring;
    xdbc_ring_t out_ring;
    xdbc_ring_t in_ring;
} xdbc_context;


int xdbc_init(xdbc_context* context);

// enqueues work items for writing/reading.
// this is an async execution
int xdbc_request_write(xdbc_context* context, const void* buffer, size_t size);
int xdbc_request_read(xdbc_context* context, void* buffer, size_t size);
