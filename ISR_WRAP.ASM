
; isr_wrap.asm for F474 ##TUT05           by H.C. Chen 2008/08/06 09:34:06 AM


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; PMode tutorials in C and Asm                          ;;
;; Copyright (C) 2000 Alexei A. Frounze                  ;;
;; The programs and sources come under the GPL           ;;
;; (GNU General Public License), for more information    ;;
;; read the file gnu-gpl.txt (originally named COPYING). ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

GLOBAL  _isr_00_wrapper, _isr_01_wrapper, _isr_02_wrapper, _isr_03_wrapper
GLOBAL  _isr_04_wrapper, _isr_05_wrapper, _isr_06_wrapper, _isr_07_wrapper
GLOBAL  _isr_08_wrapper, _isr_09_wrapper, _isr_0A_wrapper, _isr_0B_wrapper
GLOBAL  _isr_0C_wrapper, _isr_0D_wrapper, _isr_0E_wrapper, _isr_0F_wrapper
GLOBAL  _isr_10_wrapper, _isr_11_wrapper, _isr_12_wrapper, _isr_13_wrapper
GLOBAL  _isr_14_wrapper, _isr_15_wrapper, _isr_16_wrapper, _isr_17_wrapper
GLOBAL  _isr_18_wrapper, _isr_19_wrapper, _isr_1A_wrapper, _isr_1B_wrapper
GLOBAL  _isr_1C_wrapper, _isr_1D_wrapper, _isr_1E_wrapper, _isr_1F_wrapper
GLOBAL  _isr_20_wrapper, _isr_21_wrapper

GLOBAL  _exc_has_error, _isr_wrapper_table ; H.c. Chen F474 2008/08/06 09:31:23 AM

EXTERN  _exc_handler   ; _timer_handler, _kbd_handler  /* ##exc_handler can handle timer_handler and kbd_handler H.C. 2008/08/19 10:48:58 AM */

SEGMENT _TEXT PUBLIC CLASS=CODE USE16

%macro  WRAPPER 2
%1:
        push    ax
        mov     ax, %2                  ; save exception number
        jmp     _exc_common             ; jump to the common code
%endmacro

_exc_common:
        push    bp
        mov     bp, sp
        push    ds
        push    es                      ; saving segment registers and
        pushad                          ; other regs because it's an ISR
        mov     bx, 10h
        mov     ds, bx
        mov     es, bx                  ; load ds and es with valid selector
        mov     bx, ax
        cmp     byte [_exc_has_error+bx], 0
        je      .1
        push    word [ss:bp+4]          ; error code
;       push    dword [ss:bp+8]         ; ip
;       push    word [ss:bp+12]         ; cs
        push    word [ss:bp+6]         ; ip    H.C. Chen 2008/08/06 09:29:25 AM   I use 16 bits segment
        push    word [ss:bp+8]          ; cs    H.C. Chen 2008/08/06 09:29:30 AM
        jmp     .2
.1:
        push    word 0                  ; error code
        push    word [ss:bp+4]          ; ip  my segment is 16 bits
;       push    word [ss:bp+8]          ; cs    H.C. Chen 2008/08/06 09:29:25 AM   I use 16 bits segment
        push    word [ss:bp+6]          ; cs
.2:
        push    ax                      ; exception no
; void exc_handler (no, cs, ip, error)
        call    _exc_handler            ; call actual ISR code

;       add     sp, 10
        add     sp, 8     ; h.c. chen 2008/926 my segment is 16 bits so as to compatible F interpreter

        popad                           ; restoring the regs
        pop     es
        pop     ds
        pop     bp
        pop     ax

;        iretd
        iret           ; h.c. chen 2008/9/26 my code segment is 16 bits.

WRAPPER _isr_00_wrapper, 00h
WRAPPER _isr_01_wrapper, 01h
WRAPPER _isr_02_wrapper, 02h
WRAPPER _isr_03_wrapper, 03h
WRAPPER _isr_04_wrapper, 04h
WRAPPER _isr_05_wrapper, 05h
WRAPPER _isr_06_wrapper, 06h
WRAPPER _isr_07_wrapper, 07h
WRAPPER _isr_08_wrapper, 08h
WRAPPER _isr_09_wrapper, 09h
WRAPPER _isr_0A_wrapper, 0Ah
WRAPPER _isr_0B_wrapper, 0Bh
WRAPPER _isr_0C_wrapper, 0Ch
WRAPPER _isr_0D_wrapper, 0Dh
WRAPPER _isr_0E_wrapper, 0Eh
WRAPPER _isr_0F_wrapper, 0Fh
WRAPPER _isr_10_wrapper, 10h
WRAPPER _isr_11_wrapper, 11h
WRAPPER _isr_12_wrapper, 12h
WRAPPER _isr_13_wrapper, 13h
WRAPPER _isr_14_wrapper, 14h
WRAPPER _isr_15_wrapper, 15h
WRAPPER _isr_16_wrapper, 16h
WRAPPER _isr_17_wrapper, 17h
WRAPPER _isr_18_wrapper, 18h
WRAPPER _isr_19_wrapper, 19h
WRAPPER _isr_1A_wrapper, 1Ah
WRAPPER _isr_1B_wrapper, 1Bh
WRAPPER _isr_1C_wrapper, 1Ch
WRAPPER _isr_1D_wrapper, 1Dh
WRAPPER _isr_1E_wrapper, 1Eh
WRAPPER _isr_1F_wrapper, 1Fh
WRAPPER _isr_20_wrapper, 20h  ; ##exc_handler can handle timer_handler and kbd_handler H.C. 2008/08/19 10:48:58 AM
WRAPPER _isr_21_wrapper, 21h

; ##exc_handler can handle timer_handler and kbd_handler H.C. 2008/08/19 10:48:58 AM
; _isr_20_wrapper:
;         push    ds
;         push    es                      ; saving segment registers and
;         pushad                          ; other regs because it's an ISR
;         mov     bx, 10h
;         mov     ds, bx
;         mov     es, bx                  ; load ds and es with valid selector
;         call    _timer_handler          ; call actual ISR code
;         popad                           ; restoring the regs
;         pop     es
;         pop     ds
;         iretd
; 
; _isr_21_wrapper:
;         push    ds
;         push    es                      ; saving segment registers and
;         pushad                          ; other regs because it's an ISR
;         mov     bx, 10h
;         mov     ds, bx
;         mov     es, bx                  ; load ds and es with valid selector
;         call    _kbd_handler            ; call actual ISR code
;         popad                           ; restoring the regs
;         pop     es
;         pop     ds
;         iretd

SEGMENT _DATA PUBLIC CLASS=DATA
        _exc_has_error  DB      0,0,0,0,0,0,0,0, 1,0,1,1,1,1,1,0
                        DB      0,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0
                        DB      0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0

; H.C. Chen F474 08-08-07  10:08:34
    _isr_wrapper_table  DW      _isr_00_wrapper
                        DW      _isr_01_wrapper
                        DW      _isr_02_wrapper
                        DW      _isr_03_wrapper
                        DW      _isr_04_wrapper
                        DW      _isr_05_wrapper
                        DW      _isr_06_wrapper
                        DW      _isr_07_wrapper
                        DW      _isr_08_wrapper
                        DW      _isr_09_wrapper
                        DW      _isr_0A_wrapper
                        DW      _isr_0B_wrapper
                        DW      _isr_0C_wrapper
                        DW      _isr_0D_wrapper
                        DW      _isr_0E_wrapper
                        DW      _isr_0F_wrapper
                        DW      _isr_10_wrapper
                        DW      _isr_11_wrapper
                        DW      _isr_12_wrapper
                        DW      _isr_13_wrapper
                        DW      _isr_14_wrapper
                        DW      _isr_15_wrapper
                        DW      _isr_16_wrapper
                        DW      _isr_17_wrapper
                        DW      _isr_18_wrapper
                        DW      _isr_19_wrapper
                        DW      _isr_1A_wrapper
                        DW      _isr_1B_wrapper
                        DW      _isr_1C_wrapper
                        DW      _isr_1D_wrapper
                        DW      _isr_1E_wrapper
                        DW      _isr_1F_wrapper
                        DW      _isr_20_wrapper
                        DW      _isr_21_wrapper
