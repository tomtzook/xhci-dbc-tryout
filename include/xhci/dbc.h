#pragma once

#include <definition.h>
#include <environment.h>
#include <xhci/ring.h>


#pragma pack(push, 1)

typedef struct {
    volatile struct {
        uint32_t capability_id : 8;
        uint32_t next_capability_ptr : 8;
        uint32_t dc_erst_max : 5;
        uint32_t reserved0 : 11;
    } dcid;
    volatile struct {
        uint32_t reserved0 : 8;
        uint32_t db_target : 8;
        uint32_t reserved1 : 16;
    } dcdb;
    volatile struct {
        uint32_t erst_size : 16;
        uint32_t reserved0 : 16;
    } dcerstsz;
    volatile struct {
        uint64_t reserved0 : 4;
        uint64_t erst_base_address : 60;
    } dcerstba;
    volatile struct {
        uint64_t desi : 3;
        uint64_t reserved0 : 1;
        uint64_t dequeue_ptr : 60;
    } dcerdp;
    volatile union {
        volatile struct {
            uint32_t dbc_run : 1;
            uint32_t lse : 1;
            uint32_t hot : 1;
            uint32_t hit : 1;
            uint32_t drc : 1;
            uint32_t reserved0 : 11;
            uint32_t debug_max_burst_size : 8;
            uint32_t device_address : 7;
            uint32_t dce : 1;
#define DBC_DCCTRL_DCE_BIT BIT(31)
        } bits;
        volatile uint32_t all;
    } dcctrl;
    volatile struct {
        uint32_t er : 1;
        uint32_t sbr : 1;
        uint32_t reserved0 : 22;
        uint32_t debug_port_number : 8;
    } dcst;
    volatile struct {
        uint32_t ccs : 1;
        uint32_t ped : 1;
        uint32_t reserved0 : 2;
        uint32_t pr : 1;
        uint32_t pls : 4;
        uint32_t reserved1: 1;
        uint32_t port_speed : 4;
        uint32_t reserved2 : 3;
        uint32_t csc : 1;
        uint32_t reserved3 : 3;
        uint32_t prc : 1;
        uint32_t plc : 1;
        uint32_t cec : 1;
        uint32_t reserved4 : 8;
    } dcportsc;
    volatile struct {
        uint64_t reserved0 : 4;
        uint64_t context_ptr : 60;
    } dccp;
    volatile struct {
        uint32_t protocol : 8;
        uint32_t reserved0 : 8;
        uint32_t vendor_id : 16;
    } dcddi1;
    volatile struct {
        uint32_t product_id : 16;
        uint32_t device_revision : 16;
    } dcddi2;
} xhci_dbc_register_t;

typedef struct {
    uint64_t reserved0 : 1;
    uint64_t address : 63;
} xhci_dbc_string_descriptor_addr_t;

typedef struct {
    xhci_dbc_string_descriptor_addr_t string0;
    xhci_dbc_string_descriptor_addr_t manufacturer_string;
    xhci_dbc_string_descriptor_addr_t product_string;
    xhci_dbc_string_descriptor_addr_t serial_number_string;
    struct {
        uint8_t string0;
        uint8_t manufacturer_string;
        uint8_t product_string;
        uint8_t serial_number_string;
    } length;
    uint32_t reserved0[7];
} xhci_dbc_info_context_t;

typedef struct {
    struct {
        uint32_t ignored : 32;
        uint32_t reserved0 : 1;
        uint32_t cerr : 2;
        uint32_t ep_type : 3;
        uint32_t reserved1 : 1;
        uint32_t hid : 1;
        uint32_t max_burst_size : 8;
        uint32_t max_packet_size : 16;
    } info;
    struct {
        uint64_t dcs : 1;
        uint64_t reserved2 : 3;
        uint64_t tr_dequeue_ptr : 60;
    } dequeue;
    struct {
        uint64_t average_trb_length : 16;
        uint64_t max_esit_payload_lo : 16;
    } tx_info;
    uint32_t reserved3[11];
} xhci_dbc_endpoint_context_t;

typedef struct {
    xhci_dbc_info_context_t info;
    xhci_dbc_endpoint_context_t out;
    xhci_dbc_endpoint_context_t in;
} xhci_dbc_context_t;

#pragma pack(pop)

typedef enum {
    DBC_DOORBELL_EP_OUT = 0,
    DBC_DOORBELL_EP_IN = 1,
} xhci_dbc_doorbell_t;
