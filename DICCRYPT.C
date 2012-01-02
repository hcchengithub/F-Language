/*
**  diccrypt.c
**
*/
#include    <stdio.h>
#include    <string.h>
#include    <bios.h>
#include    <dos.h>
#include    "undocdos.h"
#include    "forth.h"

FLSC dic_script(char *token)
{
    unsigned i, j, k;

    if (str_compare(token,"E")==0) {
        for (i=0; i<sbuf.l; i++){
            if (sbuf.b[i]=='\n') break;  /* CR LF */
        }
        for (i+=1; i<sbuf.l; i++) {
            sbuf.b[i] = sbuf.b[i] ^ cryptogram;
        }
        return OK;
    }
    if (str_compare(token,"S")==0||str_compare(token,".SCRIPT")==0) {
        j = k = 0;
        puts("");
        for (i=0; i<sbuf.l; i++){
            fputc(sbuf.b[i], stdout);
            if (sbuf.b[i]=='\n'){       /* line boundary */
                 if (j!=999) j += 1;
                 k = 0;
            }
            if (k >= 78) {                       /* cut long lines */
                while(sbuf.b[i] != '\n') i++;
                fputc('\n', stdout);
                if (j!=999) j += 1;
                k = 0;
            } else {
                k+=1;
            }
            if (j==23) {                 /* repeat the last line */
                j = getch();
                if (j==27) goto StopListing;    /* Escape to stop listing */
                if (j==13) j = 999; else j = 0;   /* none stop dump to the end */
            }
        }
        StopListing:
        printf(
            "\n"
            "Script buffer : %04X:%04X\n"
            "Script length : %04X\n"
            , _DS, sbuf.b, sbuf.l
        );
        return OK;
    }
    return FAIL; /* Unknown or SYNTAX */
}

