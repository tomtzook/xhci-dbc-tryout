
#include "definitions.h"
#include "error_handling.h"
#include "utils.h"
#include "dbc.h"


int xhci_enable_dbc(volatile xhci_dbc_register_t* reg) {
    wmb();
    reg->dcctrl.bits.dce = 1;
    wmb();
    RETURN_ON_ERROR(wait_for_set(&reg->dcctrl, DBC_DCCTRL_DCE_BIT, 1));

    wmb();
    reg->dcportsc.ped = 1;
    wmb();

    RETURN_ON_ERROR(wait_for_set(&reg->dcctrl, DBC_DCCTRL_DCR_BIT, 1));

    return ERROR_SUCCESS;
}

int xhci_disable_dbc(volatile xhci_dbc_register_t* reg) {
    reg->dcportsc.ped = 0;
    wmb();
    reg->dcctrl.bits.dce = 0;

    RETURN_ON_ERROR(wait_for_set(&reg->dcctrl, DBC_DCCTRL_DCE_BIT, 1));

    return ERROR_SUCCESS;
}
