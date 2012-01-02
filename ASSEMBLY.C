/*
**  Assembly.c
**  H.C. Chen 2008/07/07 08:27:08 AM, inline assembly bits put all together
**
*/

#pragma     inline
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <dos.h>
#include    <dir.h>
#include    <io.h>
#include    "undocdos.h"
#include    "forth.h"
#include    "pm_defs.h"

#if 0  /* 586.f06 has supported ##setup_GDT_entry and ##setup_IDT_entry */
DESCR_SEG *gdt_entry;    /* pointer to a GDT entry */
DESCR_INT *idt_entry;    /* pointer to a IDT entry */
#endif 

struct BCOMMA    bb;     /* binary buffer control structure bb.here bb.segment bb.offset */

FLSC dic_assembly(char *token)
{
    unsigned i, j, k, l, m; /* temp variables      */
    unsigned long d1, d2;    /* long temp variables */
    unsigned char far *fpb;  /* far pointer to assembly code b, buffer */ 

    if (str_compare(token,"oo")==0) {  /* [index.a index.d data.a data.d - ] */
        i = pop();
        j = pop();
        k = pop();
        l = pop();
        asm pushf       /* F45 H.C. Chen 2008/07/06 AM 10:03:17 */
        asm cli
        outportb(l,k);
        outportb(j,i);
        asm popf
        return OK;
    }
    if (str_compare(token,"oi")==0) {  /* [index.a index.d data.a - data.d ] */
        j = pop();
        k = pop();
        l = pop();
        asm pushf       /* F45 H.C. Chen 2008/07/06 AM 10:03:17 */
        asm cli
        outportb(l,k);
        i = inportb(j);
        asm popf
        push (i);
        return OK;
    }
    if (str_compare(token,"read_msw")==0) {  /* [ - msw ] */
        asm .386p
        asm smsw    ax
        push(_AX);
        return OK;
    }
    if (str_compare(token,"read_cr0")==0) {  /* [ - CR0_high CR0_low] stacked spec unified since F475 */
        asm .386p
        asm mov     eax, cr0
        asm push    ax              /* save low word */
        asm shr     eax, 16
        push(_AX);                  /* high word */
        asm pop     ax              /* low word */
        push(_AX);
        return OK;
    }
    if (str_compare(token,"write_cr0")==0) {  /* [CR0_high CR0_low - ] stacked spec unified since F475 */
        asm .386p
        _AX = pop();
        asm push    ax
        _AX = pop();
        asm shl     eax, 16
        asm pop     ax
        asm mov     cr0, eax
        return OK;
    }
    if (str_compare(token,"write_gdtr")==0) {  /* [addr - ] write to GDTR from the given GDT structure's near address in DS */
        _BX = pop();
        asm lgdt    [bx] /* index addressing mode must use BX */
        return OK;
    }
    if (str_compare(token,"read_gdtr")==0) {  /* [addr - ] read GDTR to the given near address in DS */
        _BX = pop();
        asm sgdt    [bx]
        return OK;
    }
    if (str_compare(token,"write_idtr")==0) {  /* [addr - ] write to IDTR from the given GDT structure's near address in DS */
        _BX = pop();
        asm lidt    [bx] /* index addressing mode must use BX */
        return OK;
    }
    if (str_compare(token,"read_idtr")==0) {  /* [addr - ] read IDTR to the given near address in DS */
        _BX = pop();
        asm sidt    [bx]
        return OK;
    }
    if (str_compare(token,"update_cs")==0) {  /* [CS - ] Alexei A. Frounze's method */
        _AX = pop();
        asm push    ax                /* push new segment */
        asm push OFFSET update_cs_1   /* push new offset */
        asm retf                      /* we have a new cs now */
        asm update_cs_1:
        return OK;
    }
    if (str_compare(token,"pushfd")==0) {  /* [ - high low ] get 32 bits CPU flags, necessary for V86 mode check F54 */
        asm .386p
        asm pushfd
        asm pop     eax
        asm push    ax              /* save low word */
        asm shr     eax, 16
        push(_AX);                  /* high word */
        asm pop     ax              /* low word */
        push(_AX);
        return OK;
    }

#if 0  /* 586.f06 has supported ##setup_GDT_entry and ##setup_IDT_entry */
    /* [ base_low base_high limit_low limit_high access attribute &gdt(n) - ] */
    if (str_compare(token,"setup_GDT_entry")==0) {
        gdt_entry = (DESCR_SEG *) pop(); /* address of the GDT entry */
        i = pop();                 /* byte 6 high nibble, attribute */
        j = pop();                 /* byte 5, access    */
        k = pop();                 /* byte 6 low nibble=limit_high, bit 19~16 of 20 bits limit, length - 1 */
        gdt_entry->attribs = i | k;     /* byte 6 bits 3~0, limit's bits 19~16, bit7=G, bit6=0=16bit, bit5=0, bit4=user define */
        gdt_entry->access = j;          /* byte 5 */
        gdt_entry->limit = pop();       /* byte 1~0, limit's bit 15~0 */
        i = pop();                 /* base_high */
        j = pop();                 /* base_low  */
        gdt_entry->base_h = i >> 8;     /* byte 7 base's bit 31~24 */
        gdt_entry->base_l = j;          /* byte 3~2, base's low 16 bits */
        gdt_entry->base_m = i & 0xFF;   /* byte 4, base's bit 23~16 */
        return OK;
    }

    /* [selector offset_low offset_high access param_cnt &idt(n) - ] */
    if (str_compare(token,"setup_IDT_entry")==0) {
        idt_entry = (DESCR_INT *) pop(); /* address of the IDT entry */
        i = pop();                      /* byte 4 param_cnt */
        j = pop();                      /* byte 5 access    */
        k = pop();                      /* byte 7-6 offset high */
        idt_entry->param_cnt = i;
        idt_entry->access = j;
        idt_entry->offset_h = k;
        i = pop();                      /* byte 1-0 offset low */
        j = pop();                      /* byte 3-2 selector   */
        idt_entry->offset_l = i;
        idt_entry->selector = j;
        return OK;
    }
#endif

    if (str_compare(token,"GETREGS")==0) {
        iregs.x.ax = _SP;
        sregs.cs   = _CS;
        sregs.ds   = _DS;
        sregs.es   = _ES;
        sregs.ss   = _SS;
        return OK;
    }
    /* [ -- ] DS = ##DS, ES=##ES, SS=##SS, CS=##CS seamlessly continue */
    if (str_compare(token,"SETREGS")==0) {
        asm Global setregs    /* get the address in map file for debug */
        asm setregs:
        _DS = sregs.ds;
        _ES = sregs.es;
        _SS = sregs.ss;
        /* _CS = sregs.cs; */
        asm push sregs.cs             /* push new segment */
        asm push OFFSET update_cs_2   /* push new offset */
        asm retf                      /* we have a new cs now */
        asm update_cs_2:
        return OK;
    }

#if 0     /* f48 assembly ok so drop these ones */
    if (str_compare(token,"STI")==0) {
        asm Global assembly_sti  /* get the address in map file for debug */
        asm assembly_sti:
        asm sti
        return OK;
    }
    if (str_compare(token,"CLI")==0) {
        asm Global assembly_cli  /* get the address in map file for debug */
        asm assembly_cli:
        asm cli
        return OK;
    }
    if (str_compare(token,"STI_NOP_CLI")==0) {
        asm Global assembly_sti_cli  /* get the address in map file for debug */
        asm assembly_sti_cli:
        asm sti
        asm nop
        asm nop
        asm nop
        asm nop
        asm nop
        asm nop
        asm nop
        asm nop
        asm cli
        return OK;
    }
#endif
    
    if (str_compare(token,"INT3")==0) {
        asm Global assembly_INT3  /* get the address in map file for debug */
        asm assembly_INT3:
        asm int 3
        return OK;
    }
    if (str_compare(token,"id")==0) {      /* [index -- high low] 32 bits I/O input. Stack spec unified since F474 */
        asm Global assembly_id          /* get the address in map file for debug */
        asm assembly_id:
        _DX = pop();
        asm in   eax,dx
        asm push ax                     /* low word */
        asm shr  eax,16
        push(_AX);                      /* high word */
        asm pop  ax
        push(_AX);
        return OK;
    }
    if (str_compare(token,"od")==0) {      /* [index high low -- ] 32 bits I/O output. Stack spec unified since F474 */
        asm Global assembly_od          /* get the address in map file for debug */
        asm assembly_od:
        i = pop();   /* low   */
        j = pop();   /* high   */
        k = pop();   /* index */
        asm mov ax,j
        asm shl eax,16
        asm mov ax,i
        asm mov dx,k
        asm out dx,eax
        return OK;
    }
    /* [descriptor addr_high addr_low -- high low] 32 bits memory read, F474 unified stack SPEC */
    if (str_compare(token,"fpeekd")==0) {
        asm Global assembly_fpeekd     /* get the address in map file for debug */
        asm assembly_fpeekd:
        i = pop();                     /* addr low      */
        j = pop();                     /* addr high     */
        k = pop();                     /* descriptor    */
        asm mov es,k
        asm mov bx,j
        asm shl ebx,16
        asm mov bx,i
        asm mov eax,es:[ebx]
                /* F474 unified spec [... -- high low]  */
                asm push ax
        asm shr eax,16
        push(_AX);
        asm pop  ax
        push(_AX);
        return OK;
    }
    /* [descriptor addr_high addr_low data_high data_low -- ] 32 bits memory write, F474 unified stack SPEC */
    if (str_compare(token,"fpoked")==0) {
        asm Global assembly_fpoked      /* get the address in map file for debug */
        asm assembly_fpoked:
        _AX = pop();                    /* data low  */
        asm push ax
        _AX = pop();                    /* data high   */
        asm shl eax,16
        asm pop ax
        asm mov DWORD PTR d1,eax        /* 32 bits data  */

        i = pop();                     /* addr low      */
        j = pop();                     /* addr high     */
        k = pop();                     /* descriptor    */
        asm mov es,k
        asm mov bx,j
        asm shl ebx,16
        asm mov bx,i                   /* 32 bits address */
        asm mov eax,DWORD PTR d1
        asm mov es:[ebx],eax
        return OK;
    }

    /* F48 assembly commands */
    if (str_compare(token,"b,")==0) {   /* [ byte -- ] write one byte to MK_FP(bb.segment, bb.offset) , better be done in real mode */
        asm Global assembly_b_comma  /* get the address in map file for debug */
        asm assembly_b_comma:
        fpb = MK_FP(bb.segment, bb.offset + bb.here);
        if (*fpb==' ' || *fpb=='\t' || *fpb=='\n' || *fpb=='\r' || *fpb=='\0') {   /* b, buffer boundary check. Engineer must be very sure of the code space */
            printf("Illegal assembly code space %04X:(%04X + %xh)\n", bb.segment, bb.offset, bb.here);
            return ABORT;
        } else {
            *fpb = (unsigned char) pop();
            bb.here += 1;
        }
        return OK;
    }
    if (str_compare(token,"b,here")==0) {   /* [ -- addr ] return assembly binary buffer structure address */
        asm Global assembly_b_here  /* get the address in map file for debug */
        asm assembly_b_here:
        push((unsigned) &bb.here);
        return OK;
    }
#if 0 /* F54 */    
    if (str_compare(token,"code_buffer")==0) {
        asm Global assembly_code_start  /* get the address in map file for debug */
        asm assembly_code_start:
        asm        push    OFFSET assembly_code_space
        asm        call    near ptr _push
        asm        pop     cx                        
        asm        push    OFFSET assembly_code_end
        asm        ret
        asm Global assembly_code_space  /* get the address in map file for debug */
        asm assembly_code_space:
        asm        db      'xxxxxxxxxxxxxxxx'
        asm        db      'xxxxxxxxxxxxxxxx'
        asm        db      'xxxxxxxxxxxxxxxx'
        asm        db      'xxxxxxxxxxxxxxxx'
        asm        db      'xxxxxxxxxxxxxxxx'
        asm        db      'xxxxxxxxxxxxxxxx'
        asm        db      'xxxxxxxxxxxxxxxx'
        asm        db      'xxxxxxxxxxxxxxxx ',00  /* <==== space and NUL ended for safety check */
        asm Global assembly_code_end  /* get the address in map file for debug */
        asm assembly_code_end: 
        return OK;
    }       
#endif         

#if 0
    /*  printf [ string, v1 , v2, ... n -- int ] 
        'printf' command needs a given n at TOS, number of arguments, can be 1,2,... etc 
    */
    if (str_compare(token,"printf")==0) {
        asm Global assembly_printf  /* get the address in map file for debug */
        asm assembly_printf:
        j = pop();
        for (i=0; i<j; i++){
                k=(unsigned) pop();
                asm push WORD PTR k
        }
        l = j*2;  /*  stack bytes used by input arguments, need to drop later  */
        asm call near ptr _printf   /* get sample by i = printf("%d",j,k,l); tcc -S -c assembly.c */
        asm add sp,l     /* drop input arguments from stack                 */
        push(_AX);       /* push return value i to forth data stack         */
        return OK;
    }
#endif
    
    /*  funcall [ string, v1 , v2, ...  n entry_offset-- AX DX ]  C function call
        'funcall' needs the C function offset at TOS and then a given n , number of arguments, can be 1,2,... etc 
    */
    if (str_compare(token,"funcall")==0) {
        asm Global assembly_funcall  /* get the address in map file for debug */
        asm assembly_funcall:
        m = (unsigned) pop();	
        j = (unsigned) pop();
        for (i=0; i<j; i++){
                k=(unsigned)pop();
                asm push WORD PTR k
        }
        l = j*2;  /*  stack bytes used by input arguments, need to drop later  */
        asm call WORD PTR [m]   /* get sample by i = printf("%d",j,k,l); tcc -S -c assembly.c */
        asm add sp,l     /* drop input arguments from CPU stack */
        asm mov i,DX     /* high 16 bits of possible 32 bits return value */
        push(_AX);       /* push return value AX to forth data stack */
        push(i);         /* push return value DX to forth data stack */
        return OK;
    }

    return FAIL; /* Unknown word */
}

