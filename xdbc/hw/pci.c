
#include "io.h"
#include "hw/pci.h"

// https://wiki.osdev.org/PCI
// https://elixir.bootlin.com/linux/latest/source/arch/x86/pci/early.c

uint32_t pci_read_config32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
    return inl(0xcfc);
}

uint16_t pci_read_config16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
    return inw(0xcfc + (offset & 2));
}

uint8_t pci_read_config8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
    return inw(0xcfc + (offset & 3));
}

void pci_write_config32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
    outl(value, 0xcfc);
}

void pci_write_config16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value) {
    outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
    outl(value, 0xcfc + (offset & 2));
}

void pci_write_config8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t value) {
    outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
    outl(value, 0xcfc + (offset & 3));
}

int pci_find_class(uint32_t wanted_class, uint32_t* bus_out, uint32_t* device_out, uint32_t* func_out) {
    for (int bus = 0; bus < PCI_MAX_BUSES; ++bus) {
        for (int device = 0; device < PCI_MAX_DEVICES; ++device) {
            for (int func = 0; func < PCI_MAX_FUNCTIONS; ++func) {
                uint32_t class = pci_read_config32(bus, device, func, PCI_CLASS_REVISION);
                if ((class >> 8) == wanted_class) {
                    *bus_out = bus;
                    *device_out = device;
                    *func_out = func;
                    return 1;
                }
            }
        }
    }

    return 0;
}
