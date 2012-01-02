
/*
**  Frounze.c
**  For Alexei A. Frounze's Protected Mode Tutorials
**
*/

#pragma     inline
#include    <stdio.h>
#include    <dos.h>
#include    "undocdos.h"
#include    "forth.h"
#include    "pm_defs.h"
#include    "isr_wrap.h"

#if 0    /* GDT IDT moved in to .f reserved by script file directly */
DESCR_SEG gdt[5];        /* GDT  */
DESCR_INT idt[0x22];     /* IDT */
#endif 

FLSC dic_frounze(char *token)
{
    int i, j, k, l; /* temp variables      */

#if 0 /* F48 assembly works so these codes can be removed */
    if (str_compare(token,"int21h")==0) {
        /* [--] call int 21h, like int86 but for PMode it doesn't load segment registers  */
        asm Global int21h
        asm int21h:
        asm        int     21h
        return OK;
    }
    if (str_compare(token,"int20h")==0) {
        /* [--] call int 20n, like int86 but for PMode it doesn't load segment registers  */
        asm Global int20h
        asm int20h:
        asm        int     20h
        return OK;
    }
    
    if (str_compare(token,"&isr_00_wrapper")==0) {
        /* [ -- offset] Get ISR entry offset addresses       */
        asm        .386p
        asm        push    OFFSET isr_00_wrapper
        asm        call    near ptr _push
        asm        pop     cx
        return OK;
    }
    if (str_compare(token,"&isr_01_wrapper")==0) {
        /* [ -- offset] Get ISR entry offset addresses    
            Now we have 00 and 01, we know all of them by calculation.
            &isr_n = (&isr_01 - &isr_00)*n + &isr_00  
        */
        asm        push    OFFSET isr_01_wrapper
        asm        call    near ptr _push
        asm        pop     cx
        return OK;
    }                          
    if (str_compare(token,"&exc_handler")==0) {
        asm        push    OFFSET _exc_handler
        asm        call    near ptr _push
        asm        pop     cx
        return OK;
    }
#endif 

    if (str_compare(token,"&isr_wrapper_table")==0) {
        /* [ -- address] Store all isr wrappers' entry point to the given table */
        asm        .386p
        asm        push    OFFSET isr_wrapper_table
        asm        call    near ptr _push
        asm        pop     cx
        return OK;
    }                          
    if (str_compare(token,"&exc_has_error")==0) {
        asm        push    OFFSET exc_has_error
        asm        call    near ptr _push
        asm        pop     cx
        return OK;
    }

    return FAIL; /* Unknown word */
}

/* const char exchandler[] = "##EXC_HANDLER";  f50 ss зяжи token -hcchen5600 2008/12/20 08:53 */

int exc_handler (word exc_no, word cs, word ip, word error)
{   
    push(exc_no);
    push(cs);
    push(ip);
    push(error);    /* No problem !! Turbo C is smart than me. ip_high is just two bytes away from ip_low, it knows !! */
    return one_word("##EXC_HANDLER");  /* execute the given word */
}

#if 0  /* ##exc_handler can handle timer_handler and kbd_handler H.C. 2008/08/19 10:48:58 AM  */
void timer_handler() {
  byte far *scr;
/*
  ticks++;
  scr = MK_FP (0x20, 80*4+8*2);
  scr[0]++;
  outportb (PORT_8259M, EOI);
*/
}

void kbd_handler() {
/*
  scancode = inportb (PORT_KBD_A);
  outportb (PORT_8259M, EOI);
*/
}
#endif 

