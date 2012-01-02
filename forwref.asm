
; Forward Reference Errors in Single-Pass Assembly
; Note! China government blocks the URL: http://cs.haifa.ac.il/courses/com_org/2006/forwref.pdf&ei=Wl90SLuBCJCe6gOPi5nRAQ&usg=AFQjCNEbuIq7pE8QBl3dQsWV88aBJrQtlQ&sig2=je4D8AKbVFdFc11QYi2xDg
; Turbo C 2.0, TASM 1.0 error : "Forward reference needs override", I searched the net and found this paper. Note! China government blocks the URL. 

; this program forwref.asm demostrates forward reference errors in single-pass assembly

            .model  small
            .code
knownPtr    dd      ?
knownVar    dw      ?

          ; forward jump: distance to target is unknwn
            jmp     veryClose
            jmp     short veryClose
            
          ; indirect jump: needs pointer size (near/far)
            jmp     [unknownPtr]                    ; **Error** FORWREF.ASM(18) Forward reference needs override
            jmp     dword ptr [unknownPtr]
            jmp     [knownPtr]
            
          ; variable in code segment: needs segment override
veryClose:  mov     unknownVar,1234h                ; **Error** FORWREF.ASM(23) Forward reference needs override
            mov     cs:unknownVar,1234h
            mov     knownVar,1234h
            
          ; proc in data segment: needs segment override
            call    [procPtr]                       ; **Error** FORWREF.ASM(28) Forward reference needs override
            call    ds:[procPtr]
            
          ; far proc: needs far call
            call    testproc                        ; **Error** FORWREF.ASM(32) Forward reference needs override
            call    far ptr testproc
            
testproc    proc    far
            ret
            endp
            
unknownVar  dw      ?

            .data
procPtr     dw      ?
unknownPtr  dd      ?

            end

tasm forwref.asm 

Turbo Assembler  Version 1.0  Copyright (c) 1988 by Borland International

Assembling file:   FORWREF.ASM
**Error** FORWREF.ASM(18) Forward reference needs override
**Error** FORWREF.ASM(23) Forward reference needs override
**Error** FORWREF.ASM(28) Forward reference needs override
**Error** FORWREF.ASM(32) Forward reference needs override
Error messages:    4
Warning messages:  None
Remaining memory:  358k

D:\DOWNLOAD\200461~1\FORTH\F46>tasm /m forwref.asm
**Fatal** Bad switch: /m  <========================== TASM 4.0 supports multi-pass , all above errors will not happen.

D:\DOWNLOAD\200461~1\FORTH\F46>tasm
Turbo Assembler  Version 1.0  Copyright (c) 1988 by Borland International

Syntax:  TASM [options] source [,object] [,listing] [,xref]

/a,/s         Alphabetic or Source-code segment ordering
/c            Generate cross-reference in listing
/dSYM[=VAL]   Define symbol SYM = 0, or = value VAL
/e,/r         Emulated or Real floating-point instructions
/h,/?         Display this help screen
/iPATH        Search PATH for include files
/jCMD         Jam in an assembler directive CMD (eg. /jIDEAL)
/kh#,/ks#     Hash table capacity #, String space capacity #
/l,/la        Generate listing: l=normal listing, la=expanded listing
/ml,/mx,/mu   Case sensitivity on symbols: ml=all, mx=globals, mu=none
/n            Suppress symbol tables in listing
/p            Check for code segment overrides in protected mode
/t            Suppress messages if successful assembly
/w0,/w1,/w2   Set warning level: w0=none, w1=w2=warnings on
/w-xxx,/w+xxx Disable (-) or enable (+) warning xxx
/x            Include false conditionals in listing
/z            Display source line with error message
/zi,/zd       Debug info: zi=full, zd=line numbers only

D:\DOWNLOAD\200461~1\FORTH\F46>
