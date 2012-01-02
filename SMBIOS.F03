//---------------------------------------------------------------------------------------------------------//
//   SMBIOS Tool script vF03 (Use F54.exe) 2009/03/04 by Wistron WKS RD SW NS0K00                          //
//---------------------------------------------------------------------------------------------------------//
//   _scasd ---> @_scasd
//   remove nearcall_assembler   
//---------------------------------------------------------------------------------------------------------//
//   SMBIOS Tool script vF02 (Use F52.exe) 2009/02/24 by Wistron WKS RD SW NS0K00                          //
//---------------------------------------------------------------------------------------------------------//
//   SMBIOS Tool script vF01 (Use F52.exe) 2009/02/23 by Wistron WKS RD SW NS0K00                          //
//---------------------------------------------------------------------------------------------------------//
1.   Get_Bit_State           // [ Date,bit -- the state of the bit: 0? 1? ]

//---------------------------------------------------------------------------------------------------------

#Get_Bit_State           // [ Data,Bit -- the state of the bit: 0? 1?]
    SWAP >A  DUP $0 == IF A@ $1 & $1 ==
                    ." "               
                 ELSE  DUP $1 ==
                    IF A@  $2 & $2 ==
                       ." "
                    ELSE  DUP $2 ==
                       IF A@  $4 & $4 ==
                          ." "
                       ELSE  DUP $3 ==
                          IF A@  $8 & $8 ==
                             ." "
                          ELSE DUP $4  ==
                             IF A@ $10 & $10 ==
                                ." "
                             ELSE DUP $5 == 
                                IF A@ $20 & $20 ==
                                   ." "
                                ELSE DUP $6 ==
                                   IF A@  $40 & $40 ==
                                      ." " 
                                   ELSE DUP $7 == 
                                      IF A@  $80 & $80 ==
                                         ." "
                                      ELSE   
                                      ENDIF 
                                   ENDIF 
                                ENDIF 
                             ENDIF 
                          ENDIF 
                       ENDIF 
                    ENDIF 
                 ENDIF 
                 SWAP DROP A> DROP ; 
                                            

//---------------------------------------------------------------------------------------------------------//
//   SMBIOS Tool script vF00 (Use F52.exe) 2009/02/20 by Wistron WKS RD SW NS0K00                          //
//---------------------------------------------------------------------------------------------------------//

1.   Find_SM_                 // [ -- Addr of _SM_ ]
2.   Find_SMBios_Entry        // [ -- Seg Off (SMBios Entry)] 
3.   SMBios_Entry_Seg         // [ -- Seg (SMBios Entry)]
4.   SMBios_Entry_Off         // [ -- Off (SMBios Entry)]
5.   Get_SMBios_DataB         // [ Off -- Data ] Get data from SMBios_Entry+off (BYTE)
6.   Get_SMBios_DataW         // [ Off -- Data ] Get data from SMBios_Entry+off (WORD)
7.   ReadSMBiosString         // [ Offset Length -- Each_byte_of_string ...]
8.   ShowStackStr             // [ Length -- ]
9.   Find_String              // [ Addr -- Length_of_string Next_string_address]
10.  GetTypeInfo              // [ Base_addr -- Type_num Length_of_the_formatted_area Addr_of_string1 ]
11.  Next_Type_Addr           // [ Addr_of_type -- Next_type_addr ]
12.  Get_Type_Base            // [ Type_num -- Type_base ]
13.  Get_Type_Formatted_Data  // [ Type_num off -- Data ]
14.  Get_Type_String_Base     // [ Type_num String_num -- Base_addr_of_String_num ]
15.  Show_Type_String         // [ Type_num String_num --  ]

16.  Read_SMBIOS_Attrib_Byte   // [ Off -- Data ] Get data from _SM_Base+off (BYTE)
17.  Read_SMBIOS_Attrib_Word   // [ Off -- Data ] Get data from _SM_Base+off (WORD)

//---------------------------------------------------------------------------------------------------------

#AUTOEXEC  ." SMBIOS Tool script vF02 (Use F52.exe) 2009/02/24 by Wistron WKS RD SW NS0K00 " CR
           ." This tool for SMBIOS is slow. Please wait until it is finished ^_^!" CR
           ##assembler
           ;

#Find_SM_   // [ -- Addr of _SM_ ]
     $f000 $0 $4000 $535F $5F4D ##scasd 
     not if 0  endif ;

// #Find_SM_   // [ -- Addr of _SM_ ]
     //CR $0000 >A
//     $0000 >A
//     DO S" _S" PEEKW $F000 A@ FPEEKW - ." ."
//        IF  
//        ELSE //." Find _S anchor!" CR
//             S" M_" PEEKW $F000 A@ $2 + FPEEKW - 
//             IF 
//            ELSE A> //." Find _SM_ anchor!"
//             BREAK ENDIF
//        ENDIF 
//     +LOOP ;
// #Find_SM_   // [ -- Addr of _SM_ ]                      // Slow one
     //CR $0000 >A
//   $0000 >A
//   DO '_ 'S $100 * + $F000 A@ FPEEKW -                  //Use '_ 'S $100 * + instead of S" _S" PEEKW
//      IF  
//      ELSE //." Find _S anchor!" CR
//           'M '_ $100 * + $F000 A@ $2 + FPEEKW -        //Use 'M '_ $100 * + instead of S" M_" PEEKW
//           IF 
//           ELSE A> //." Find _SM_ anchor!"
//           BREAK ENDIF
//      ENDIF 
//   +LOOP ;

#Find_SMBios_Entry   // [ -- Seg Off (SMBios Entry)]  
     $F000 ##Find_SM_ DUP -ROT $1A + FPEEK $1000 *
     $F000 ROT $18 + FPEEKW ;

//#Find_SMBios_Entry   // [ -- Seg Off (SMBios Entry)]    // Slow one
//     $F000 ##Find_SM_ $1A + FPEEK $1000 *
//     $F000 ##Find_SM_ $18 + FPEEKW ;

#SMBios_Entry_Seg   // [ -- Seg (SMBios Entry)]
     ##Find_SMBios_Entry DROP ;

#SMBios_Entry_Off   // [ -- Off (SMBios Entry)]
     ##Find_SMBios_Entry SWAP DROP ;

#Get_SMBios_DataB   // [ Off -- Data ] Get data from SMBios_Entry+off (BYTE)
     //##Find_SMBios_Entry ROT + FPEEK DUP .$ ;
     ##Find_SMBios_Entry ROT + FPEEK ;

#Get_SMBios_DataW   // [ Off -- Data ] Get data from SMBios_Entry+off (WORD)
     //##Find_SMBios_Entry ROT + FPEEKW DUP .$ ;
     ##Find_SMBios_Entry ROT + FPEEKW ;     

#ReadSMBiosString    // [ Offset Length -- Each_byte_of_string ...]
    DO DUP 0 <= ?BREAK 01 - SWAP DUP ##Get_SMBios_DataB SWAP 01 + 02 PICK LOOP
    DROP DROP ;

#ShowStackStr    // [ Length -- ]
    PUTENV StrBuff=0123456789abcdef   //get a 16 bytes buffer
    GETENV StrBuff SWAP
    OVER OVER + 00 POKE      //write \0 to end of string
    DO DUP 0 <= ?BREAK 01 - OVER OVER + 03 PICK POKE LOOP
    DROP TYPE
    PUTENV StrBuff= ;

#Find_String   // [ Addr -- Length_of_string Next_string_address]
     $0000 >A
     DO DUP ##SMBios_Entry_Seg SWAP A@ + FPEEK $0 -
        IF  
        ELSE
            A> DUP ROT $1 + + BREAK 
        ENDIF
     +LOOP ;

#GetTypeInfo   // [ Base_addr -- Type_num Length_of_the_formatted_area Addr_of_string1 ]
    DUP ##SMBios_Entry_Seg SWAP OVER OVER $0 + FPEEK $3 PICK $3 PICK $3 PICK $1 + FPEEK SWAP OVER + ;

#Next_Type_Addr // [ Addr_of_type -- Next_type_addr ]
     ##GetTypeInfo $2 PICK DROP SWAP DROP
     $0000 >A
     DO DUP ##SMBios_Entry_Seg SWAP A@ + FPEEKW $00 -
        IF  
        ELSE
            A> DUP ROT $2 + + BREAK 
        ENDIF
     +LOOP SWAP DROP ;

#Get_Type_Base   // [ Type_num -- Type_base ]
     DUP $0 ==
     IF DROP ##SMBios_Entry_Off
     ELSE
        ##SMBios_Entry_Off >A
           DO 
           DUP A> DUP $3 PICK $2 PICK ##GetTypeInfo DROP DROP -
           IF ##Next_Type_Addr >A
           ELSE
              SWAP DROP BREAK
           ENDIF
           LOOP
     ENDIF ;

#Get_Type_Formatted_Data   // [ Type_num off -- Data ]
     SWAP ##Get_Type_Base + ##SMBios_Entry_Seg SWAP FPEEK ;

#Get_Type_String_Base   // [ Type_num String_num -- Base_addr_of_String_num ]
     SWAP ##Get_Type_Base ##GetTypeInfo SWAP DROP SWAP DROP
     SWAP DUP $1 ==
     IF DROP
     ELSE
         $1 - >A
         DO A@ $0 ==
            IF A> DROP BREAK
            ELSE
               ##Find_String SWAP DROP
            ENDIF    
         -LOOP      
     ENDIF ;

#Show_Type_String   // [ Type_num String_num --  ]
     ##Get_Type_String_Base DUP ##Find_String DROP SWAP ##SMBios_Entry_Off - SWAP DUP >A
     ##ReadSMBiosString
     A> ##ShowStackStr ;
     
#Read_SMBIOS_Attrib_Byte   // [ Off -- Data ] Get data from _SM_Base+off (BYTE)
     $F000 ##Find_SM_ ROT + FPEEK ;

#Read_SMBIOS_Attrib_Word   // [ Off -- Data ] Get data from _SM_Base+off (WORD)
     $F000 ##Find_SM_ ROT + FPEEKW ;


  ~~~~~~~~~~ funcall words (C library) ~~~~~~~~~~~~~
* #farcoreleft // [ -- low high ] check DOS's remaining memory
     00 062 map funcall ;
* #farmalloc // [ high16 low16 -- offset segment ] get DOS's memory block
     swap 02 060 map funcall ;
* #printf  000 map funcall drop drop ;  [ string arg1 arg2 ... n -- ] printf , n is the number of given arguments
* #sprintf 002 map funcall drop ; [ dest string arg1 arg2 ... n1 -- n2 ] sprintf , n1 is the number of given arguments. n2 is length

  ~~~~~~~~~~ Useful Tools ~~~~~~~~~~~~~~~~~~~~~~~~~
* #KEY? $0100 ##iAX! $16 int86 ##FL $40 & if 0 ret endif $0000 ##iAX! $16 int86 ##oAX 01 ; [-- [AX] key?]
* #max over over < if swap endif drop ; [ a b - max] a b are int (signed)
  #.seg:off s" %04x:%04x " -rot 03 ##printf ; [ seg off -- ] print seg:off
  #dump16hex  // [ seg off -- seg off' ] print 16 hex, return new offset
    016 -rot do       \ 16 seg off
    over over fpeek   \ 16 seg off c
    s" %02x " swap 02 \ 16 seg off $ c 02
    ##printf          \ 16 seg off
    01 +              \ 16 seg off++
    rot 01 - dup      \ seg off++ 16-- 16--
    dup 08 == if ." - " endif
    0 == ?break       \ seg off++ 16--
    -rot              \ 16-- seg off++
    loop drop         \ seg off'
    ;
  #emit16  // [ seg off -- seg off' ] emit 16 characters, return new offset
    016 -rot do     \ 16 seg off
    over over fpeek \ 16 seg off c
    dup 032 < if drop '. endif \ c'
    s" %1c" swap 02 \ 16 seg off $ c 02
    ##printf        \ 16 seg off
    01 +            \ 16 seg off++
    rot 01 - dup    \ seg off++ 16-- 16--
    0 == ?break     \ seg off++ 16--
    -rot            \ 16-- seg off++
    loop drop       \ seg off'
    ;
* #d // [ seg off -- seg off' ] dump 16 bytes memroy, return new offset
    over over ##.seg:off over over ."  " ##dump16hex drop drop ."  " ##emit16 cr
    over over ##.seg:off over over ."  " ##dump16hex drop drop ."  " ##emit16 cr
    ;
* #dd    08 >A do A@ not ?break ##d -loop A> DROP ;
* #ddd   ##dd ##dd ##dd ##dd ;
* #dddd  ##ddd ##ddd ##ddd ##ddd ;
* #ddddd ##dddd ##dddd ##dddd ##dddd ##dddd ##dddd ##dddd ##dddd ##dddd ##dddd ##dddd ##dddd ##dddd ##dddd ##dddd ##dddd ;

  #64k_segment s" xx " ;
  #64k_offset  s" xx " ;
* #autoexec
    ." Init descriptor tables . . . " ##init_descriptor_tables ." done" cr
    ." Allocate 64k buffer for ##xD commands . . . "
    $0001 $0010 ##farmalloc    // [high low -- fp_off fp_seg]
    ##64k_segment swap    01 + pokew // adjust to segment+1:0000
    ##64k_offset  swap $fff0 & pokew
    ##64k_segment peekw 01 == if ." failed !!" exit else ." done" cr endif
    clearcache
    ;

  ~~~~~~~~~~ farcall assembly code words (in data segment script buffer) ~~~~~~~~~~~~~

  // ##scasd used CPU SCASD instruction to find given dward data in the given real mode space.
  // [ start_seg start_off length/4 target_low target_high -- (offset true) | false ]
  // Example : f54 pmtools.f10 $f000 $0 $4000 $535F $5F4D ##scasd .s <== 肚 [ -- addr 1 ] f000:addr 狦痷 _SM_ 矪
* #scasd
    s" xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx " ##inline_assembly if
    \ $CC b,                        \ int3
      $55 b,                        \ push         bp
      $8B b, $EC b,                 \ mov          bp,sp
      $8B b, $5E b, $06 b,          \ mov          bx,[bp+06]
      $FC b,                        \ cld
      $8B b, $47 b, $F8 b,          \ mov          ax,[bx-08]
      $8E b, $C0 b,                 \ mov          es,ax
      $8B b, $7F b, $FA b,          \ mov          di,[bx-06]
      $8B b, $4F b, $FC b,          \ mov          cx,[bx-04]
      $66 b, $8B b, $47 b, $FE b,   \ mov          eax,[bx-02]
      $F2 b, $66 b, $AF b,          \ repnz scasd
      $89 b, $7F b, $F8 b,          \ mov          [bx-08],di
      $89 b, $4F b, $FA b,          \ mov          [bx-06],cx
      $5D b,                        \ pop          bp
      $CB b,                        \ retf
    endif // [ start_segment[bx-8] start_offset[bx-6] length[bx-4] target_low[bx-2] target_high[bx] -- di cx length target_low target_high ]
    farcall drop drop drop if 04 - 01 else drop 0 endif ;
  ;

  // ##MOVSB "move string byte" for real mode
  // [ from_seg[bx-8] from_off[bx-6] to_seg[bx-4] to_off[bx-2] length[bx] -- ]
  // Example : ##ds 0 ##64k_segment 0 $ffff ##MOVSB .s <== move DS to the 64k buffer
  //           ##ds $ff00 ##dd drop drop               <== check end of DS
  //           ##64k_segment peekw ##64k_offset peekw $ff00 + ##dd drop drop   <== check end of 64k buffer
  // The above example works fine ! hcchen5600 2009/03/06 11:04  
  // Τ硂皌 Apple II мォ碞Θ fill ,
  //    1. 恶璶狡碭 byte  buffer 秨繷
  //    2. from_start to_next_repeat lengh-repeat-1 ##movsb, where from_start to_next_repeat means their segment and offset
* #MOVSB
    s" xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxyyyy " ##inline_assembly if
      $CD b, $03 b,        \ int  3
      $55 b,               \ push bp
      $89 b, $E5 b,        \ mov  bp,sp
      $8B b, $5E b, $06 b, \ mov  bx,[bp+6]    ;<== TOS at [bp+6]
      $1E b,               \ push ds
      $06 b,               \ push es
      $FC b,               \ cld
      $8B b, $0F b,        \ mov  cx,[bx]      ; length
      $8B b, $77 b, $FA b, \ mov  si,[bx-6]    ; from_off
      $8B b, $7F b, $FE b, \ mov  di,[bx-2]    ; to_off
      $8B b, $47 b, $FC b, \ mov  ax,[bx-4]    ; to_seg
      $8E b, $C0 b,        \ mov  es,ax
      $8B b, $47 b, $F8 b, \ mov  ax,[bx-8]    ; from_seg
      $8E b, $D8 b,        \ mov  ds,ax        ; <== DS is the last one to be changed, or [bx] reference will be all wrong!
      $F3 b, $A4 b,        \ repz movsb
      $07 b,               \ pop  es
      $1F b,               \ pop  ds
      $5D b,               \ pop  bp
      $CB b,               \ retf    
    endif farcall
    drop drop drop drop drop 
  ;

  // ##BigRealMOVSB "move string byte" You need to enter big_real_mode in prior using this macro
  // [ from_high from_low to_high to_low length_high length_low -- ] all 3 arguments are 32 bit
  // Example : f54.exe pmtools.f10  $c $0 $10 $0 $4 $0 ##BigRealMOVSB .s <== ノ QEMU ┪ Bochs 浪琩 $100000~$13ffff 单 C000:0 ~ F000:ffff
  //           f54.exe pmtools.f10 $10 $0 $10 $1 $4 $0 ##BigRealMOVSB .s <== 俱 $100000~$140001 常Θ籔 $100000 , Apple II мォ
* #BigRealMOVSB
    swap >a >a swap >a >a swap a> a> a> a>  // high-low swap
    s" xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxyyyy " ##inline_assembly if
      // [ from_low[bx-10] from_high[bx-8] to_low[bx-6] to_high[bx-4] length_low[bx-2] length_high[bx]  -- ]
      $55 b,        \ push bp
      $89 b, $E5 b, \ mov  bp,sp
      $8B b, $5E b, \
      $06 b,        \ mov  bx,[bp+6]    ;<== TOS at [bp+6]
      $1E b,        \ push ds
      $06 b,        \ push es           ;<== 猔種!!! 盽а b,
      $FC b,        \ cld
      $66 b, $8B b, \
      $4F b, $FE b, \ mov  ecx,[bx-2]   ; length_low length_high
      $66 b, $8B b, \
      $77 b, $F6 b, \ mov  esi,[bx-10]  ; from_low from_high
      $66 b, $8B b, \
      $7F b, $FA b, \ mov  edi,[bx-6]   ; to_low to_high
      $31 b, $C0 b, \ xor  ax,ax
      $8E b, $C0 b, \ mov  es,ax
      $8E b, $D8 b, \ mov  ds,ax        ;<== 程笆 DS 玥 [bx] 把σ场岿
      $67 b,        \ a32
      $F3 b, $A4 b, \ repz movsb
      $07 b,        \ pop  es
      $1F b,        \ pop  ds
      $5D b,        \ pop  bp
      $CB b,        \ retf              ;<=== 猔種!!! ret or retf
    endif farcall
    drop drop drop drop drop drop
  ;

  // ##BigRealMOVSD "move string dword" You need to enter big_real_mode in prior using this macro
  // [ from_high from_low to_high to_low length_high length_low -- ] all 3 arguments are 32 bit
  // Example : f54.exe pmtools.f10  $c $0 $10 $0 $1 $0 ##BigRealMOVSD .s <== ノ QEMU ┪ Bochs 浪琩 $100000~$13ffff 单 C000:0 ~ F000:ffff
  //           f54.exe pmtools.f10 $10 $0 $10 $4 $1 $0 ##BigRealMOVSD .s <== 俱 $100000~$140003 常Θ籔 $100000 , Apple II мォ
* #BigRealMOVSD
    swap >a >a swap >a >a swap a> a> a> a>  // high-low swap
    s" xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxyyyy " ##inline_assembly if
      // [ from_low[bx-10] from_high[bx-8] to_low[bx-6] to_high[bx-4] length_low[bx-2] length_high[bx]  -- ]
      $55 b,        \ push bp
      $89 b, $E5 b, \ mov  bp,sp
      $8B b, $5E b, \
      $06 b,        \ mov  bx,[bp+6]    ;<== TOS
      $1E b,        \ push ds
      $06 b,        \ push es           ;<== 猔種!!! 盽а b,
      $FC b,        \ cld
      $66 b, $8B b, \
      $4F b, $FE b, \ mov  ecx,[bx-2]   ; length_low length_high
      $66 b, $8B b, \
      $77 b, $F6 b, \ mov  esi,[bx-10]  ; from_low from_high
      $66 b, $8B b, \
      $7F b, $FA b, \ mov  edi,[bx-6]   ; to_low to_high
      $31 b, $C0 b, \ xor  ax,ax
      $8E b, $C0 b, \ mov  es,ax
      $8E b, $D8 b, \ mov  ds,ax        ;<== 程笆 DS 玥 [bx] 把σ场岿
      $67 b, $F3 b, \ a32 repz
      $66 b, $A5 b, \ movsd
      $07 b,        \ pop  es
      $1F b,        \ pop  ds
      $5D b,        \ pop  bp
      $CB b,        \ retf              ;<=== 猔種!!! ret or retf
    endif farcall
    drop drop drop drop drop drop
  ;

  ~~~~~~~~~~ B-COMMA Assembler commands ~~~~~~~~~~~~~~~~~~~~~~~~~
  #&b,segment b,here 04 + ; [ -- addr ]
  #&b,offset  b,here 02 + ; [ -- addr ]
  #b,segment! ##&b,segment swap pokew ; [ seg -- ]
  #b,offset!  ##&b,offset  swap pokew ; [ off -- ]
  #b,segment  ##&b,segment peekw ; [ -- seg ]
  #b,offset   ##&b,offset  peekw ; [ -- off ]
  #b,here@    b,here peekw ; [ -- here ]
  #b,here!    b,here swap pokew ;  [ here -- ]
  #b,setup    0 ##b,here! ##b,offset! ##b,segment! ;   // [ seg offset -- ] initialize b, binary code compiler
* #inline_assembly  ##xDS swap over over ##b,setup dup peek $78 == ; [ offset -- ##xDS entry first_time? ]

  ~~~~~~~~~~~~ INT86, INT86X Registers ~~~~~~~~~~~~~~~~~~~~~~~~~
  #iAL iregs 00 + peek ; #iAH iregs 01 + peek ; #iBL iregs 02 + peek ; #iBH iregs 03 + peek ;
  #iCL iregs 04 + peek ; #iCH iregs 05 + peek ; #iDL iregs 06 + peek ; #iDH iregs 07 + peek ;
  #iAX iregs 00 + peekw ; #iBX iregs 02 + peekw ; #iCX iregs 04 + peekw ; #iDX iregs 06 + peekw ;
  #iSI iregs 08 + peekw ; #iDI iregs $A + peekw ; #oAL oregs 00 + peek ; #oAH oregs 01 + peek ;
  #oBL oregs 02 + peek ; #oBH oregs 03 + peek ; #oCL oregs 04 + peek ; #oCH oregs 05 + peek ;
  #oDL oregs 06 + peek ; #oDH oregs 07 + peek ; #oAX oregs 00 + peekw ; #oBX oregs 02 + peekw ;
  #oCX oregs 04 + peekw ; #oDX oregs 06 + peekw ; #oSI oregs 08 + peekw ; #oDI oregs $A + peekw ;
  #CY oregs $C + peek ; #FL oregs $E + peekw ; #ES sregs 00 + peekw ; #CS sregs 02 + peekw ;
  #SS sregs 04 + peekw ; #DS sregs 06 + peekw ; #iAL! iregs 00 + swap poke ; #iAH! iregs 01 + swap poke ;
  #iBL! iregs 02 + swap poke ; #iBH! iregs 03 + swap poke ; #iCL! iregs 04 + swap poke ; #iCH! iregs 05 + swap poke ;
  #iDL! iregs 06 + swap poke ; #iDH! iregs 07 + swap poke ; #iSI! iregs 08 + swap pokew ; #iDI! iregs $A + swap pokew ;
  #iAX! iregs 00 + swap pokew ; #iBX! iregs 02 + swap pokew ; #iCX! iregs 04 + swap pokew ; #iDX! iregs 06 + swap pokew ;
  #ES! sregs 00 + swap pokew ; #CS! sregs 02 + swap pokew ; #SS! sregs 04 + swap pokew ; #DS! sregs 06 + swap pokew ;
  #xDS read_msw 01 & pushfd drop 02 & 02 ^ AND if $40 else ##DS endif ; // PMode  executable DS ﹚ GDT 40h
