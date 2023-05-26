
#include "definitions.h"
#include "error_handling.h"
#include "utils.h"
#include "dbc.h"


int xhci_enable_dbc(xhci_dbc_register_t* reg) {
    int status = ERROR_SUCCESS;

    wmb();
    reg->dcctrl.bits.dce = 1;
    wmb();
    GOTO_CLEAN_ON_ERROR(wait_for_set(&reg->dcctrl, DBC_DCCTRL_DCE_BIT, 1));

    wmb();
    reg->dcportsc.ped = 1;
    wmb();

    GOTO_CLEAN_ON_ERROR(wait_for_set(&reg->dcctrl, DBC_DCCTRL_DCR_BIT, 1));

clean:
    return status;
}

int xhci_disable_dbc(xhci_dbc_register_t* reg) {
    int status = ERROR_SUCCESS;

    reg->dcportsc.ped = 0;
    wmb();
    reg->dcctrl.bits.dce = 0;

    GOTO_CLEAN_ON_ERROR(wait_for_set(&reg->dcctrl, DBC_DCCTRL_DCE_BIT, 1));

clean:
    return status;
}
