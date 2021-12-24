#pragma once

#include <definition.h>

#define PCI_BASE_ADDRESS_0 (0x10)
#define PCI_COMMAND (0x4)

#define PCI_BAR_MEM_ADDRESS_SHIFT (16)

typedef union {
    struct {
        uint16_t ignored0 : 1;
        uint16_t mem_space : 1;
        uint16_t ignored1 : 14;
    } bits;
    uint16_t data;
} pci_command_t;

typedef enum {
    PCI_BAR_32_ADDRESS = 0x0,
    PCI_BAR_64_ADDRESS = 0x2,
} pci_bar_type_t;

typedef union {
    struct {
        uint32_t reserved0 : 1;
        uint32_t type : 2;
        uint32_t prefetchable : 1;
        uint32_t address : 28;
    } bits;
    uint32_t data;
} pci_bar_t;


uint32_t pci_read_config32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_read_config16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint8_t pci_read_config8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

void pci_write_config32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
void pci_write_config16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value);
void pci_write_config8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t value);
