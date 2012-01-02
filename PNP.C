#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "undocdos.h"
#include "forth.h"
#include "pnp.h"

struct PnP_BIOS_HEADER far * PnPheader;
int (far * PnPreal)();

void NullFunction()
{
    puts("Fatal: You're calling a null function!\n");
    exit(1);
}

void InstallationCheck (void)
{   unsigned       i, j;
    unsigned char  sum; /* modify Randy 98/6/30 */
    /* [--] */
    char far *pp=0xF0000000L;
    for (i=0; i<0xfff0; i+=16) {
        if ( pp[i+0]=='$' && pp[i+1]=='P' && pp[i+2]=='n' && pp[i+3]=='P' ) {
			PnPheader = (struct PnP_BIOS_HEADER far *) (pp + i);
            PnPreal = MK_FP(PnPheader->uRealSeg, PnPheader->uRealOff);
            for (sum=j=0; j<PnPheader->cLength; j++) /* modify Randy 98/6/30 */
                sum += ((BYTE far *) PnPheader)[j]; /* modify Randy 98/6/30 */
            if (sum==0) break;
        }
    }
    if( sum ) {  /* modify Randy 98/6/30 */
        puts("InstallationCheck: No PnP BIOS!\n");
        exit(1);
    }
}

int dic_PnP()
{
    unsigned err;
    if (str_compare(ss,"InstallationCheck") == 0) {
        /* [-- $PnP_offset checksum] checksum==0 OK */
        InstallationCheck();
        return OK;
    }
    if (str_compare(ss,"PnPreal") == 0) {
        /* [a b c d e f g h -- err] */
        err = (*PnPreal)(pop(),pop(),pop(),pop(),pop(),pop(),pop(),pop());
        push(err);
        return OK;
    }
    if (str_compare(ss,"Buffer") == 0) {
        /* [ -- offset] */
        push ((unsigned) Buffer);
        return OK;
    }
    if (str_compare(ss,"Buffer!") == 0) {
        /* [offset --] */
        if (Buffer!=NULL) {
            puts("Fatal: Buffer! overwrite");
            exit(1);
        }
		Buffer = (unsigned char *) pop();
        return OK;
    }
    if (str_compare(ss,"Buffer!!") == 0) {
        /* [offset --] */
		Buffer = (unsigned char *) pop();
        return OK;
    }
    if (str_compare(ss,"&PnPheader") == 0) {
        /* get PnPheader offset */
        /* [ -- off]             */
        push ((WORD) PnPheader);
        return OK;
    }
    return FAIL;  /* Unknown word */
}
