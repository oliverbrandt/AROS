/*
    Copyright © 2010, The AROS Development Team. All rights reserved
    $Id$
*/

#define DEBUG 1
#include <aros/debug.h>

#include LC_LIBDEFS_FILE

/*
    Call every HBA function with pointer to ahci_hba_chip structure as a first argument
*/

/*
    Set the AHCI HBA to a minimally initialized state.

        * Indicate that the system is AHCI aware by setting bit GHC_AE in HBA's GHC register
        * Determine number of ports implemented by the HBA
        * Ensure that all implemented ports are in IDLE state
        * Determine how many command slots the HBA supports, by reading CAP.NCS
        * Set CAP.S64A to ‘0’, no 64bit address support for now...
        * For each implemented port, allocate memory for and program registers:
            - PxCLB (and PxCLBU=0, upper 32bits of 64bit address space)
            - PxFB (and PxFBU=0, upper 32bits of 64bit address space)
        * For each implemented port, clear the PxSERR register
        * Allocate and initialize interrupts, and set each implemented port’s PxIE
            register with the appropriate enables. To enable the HBA to generate interrupts, system
            software must also set GHC.IE to a ‘1’.
*/
void ahci_init_hba(struct ahci_hba_chip *hba_chip) {
    D(bug("[AHCI] (%04x:%04x) HBA init...\n", hba_chip->VendorID, hba_chip->ProductID));

    struct ahci_hba *hba;
    hba = hba_chip->abar;

    D(bug("[AHCI] GHC = %08x", hba->ghc));
    ahci_enable_hba(hba_chip);
    D(bug("[AHCI] GHC = %08x", hba->ghc));
    ahci_disable_hba(hba_chip);
    D(bug("[AHCI] GHC = %08x", hba->ghc));
    ahci_enable_hba(hba_chip);
    D(bug("[AHCI] GHC = %08x", hba->ghc));

}

void ahci_reset_hba(struct ahci_hba_chip *hba_chip) {
    D(bug("[AHCI] (%04x:%04x) HBA reset...\n", hba_chip->VendorID, hba_chip->ProductID));
}

BOOL ahci_enable_hba(struct ahci_hba_chip *hba_chip) {
    D(bug("[AHCI] (%04x:%04x) HBA enable...\n", hba_chip->VendorID, hba_chip->ProductID));

    struct ahci_hba *hba;
    hba = hba_chip->abar;

    hba->ghc |= GHC_AE;

    return ((hba->ghc && GHC_AE) ? TRUE:FALSE);
}

void ahci_disable_hba(struct ahci_hba_chip *hba_chip) {
    D(bug("[AHCI] (%04x:%04x) HBA disable...\n", hba_chip->VendorID, hba_chip->ProductID));

    struct ahci_hba *hba;
    hba = hba_chip->abar;

    hba->ghc &= ~GHC_AE;

}
