/*
  TUT04
  PMode tutorials in C and Asm
  Copyright (C) 2000 Alexei A. Frounze
  The programs and sources come under the GPL 
  (GNU General Public License), for more information
  read the file gnu-gpl.txt (originally named COPYING).
*/

#ifndef _isr_wrap_h_
#define _isr_wrap_h_

extern  unsigned char exc_has_error[0x20];
extern  unsigned isr_wrapper_table[0x21];  /* H.c. Chen F473 2008-08-05  09:15:52 */

interrupt isr_00_wrapper();
interrupt isr_01_wrapper();
interrupt isr_02_wrapper();
interrupt isr_03_wrapper();
interrupt isr_04_wrapper();
interrupt isr_05_wrapper();
interrupt isr_06_wrapper();
interrupt isr_07_wrapper();
interrupt isr_08_wrapper();
interrupt isr_09_wrapper();
interrupt isr_0A_wrapper();
interrupt isr_0B_wrapper();
interrupt isr_0C_wrapper();
interrupt isr_0D_wrapper();
interrupt isr_0E_wrapper();
interrupt isr_0F_wrapper();
interrupt isr_10_wrapper();
interrupt isr_11_wrapper();
interrupt isr_12_wrapper();
interrupt isr_13_wrapper();
interrupt isr_14_wrapper();
interrupt isr_15_wrapper();
interrupt isr_16_wrapper();
interrupt isr_17_wrapper();
interrupt isr_18_wrapper();
interrupt isr_19_wrapper();
interrupt isr_1A_wrapper();
interrupt isr_1B_wrapper();
interrupt isr_1C_wrapper();
interrupt isr_1D_wrapper();
interrupt isr_1E_wrapper();
interrupt isr_1F_wrapper();
interrupt isr_20_wrapper();
interrupt isr_21_wrapper();

#endif
