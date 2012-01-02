/*
  PMode tutorials in C and Asm
  Copyright (C) 2000 Alexei A. Frounze
  The programs and sources come under the GPL 
  (GNU General Public License), for more information
  read the file gnu-gpl.txt (originally named COPYING).
*/

#ifndef _pm_defs_
#define _pm_defs_

#define byte unsigned char
#define word unsigned int
#define dword unsigned long

/* Access byte's flags */
#define ACS_PRESENT     0x80            /* present segment */
#define ACS_CSEG        0x18            /* code segment */
#define ACS_DSEG        0x10            /* data segment */
#define ACS_CONFORM     0x04            /* conforming segment */
#define ACS_READ        0x02            /* readable segment */
#define ACS_WRITE       0x02            /* writable segment */
#define ACS_IDT         ACS_DSEG        /* segment type is the same type */
#define ACS_INT_GATE    0x06            /* int gate for 286 */
#define ACS_INT         (ACS_PRESENT | ACS_INT_GATE) /* present int gate */

/* Ready-made values */
#define ACS_CODE        (ACS_PRESENT | ACS_CSEG | ACS_READ)
#define ACS_DATA        (ACS_PRESENT | ACS_DSEG | ACS_WRITE)
#define ACS_STACK       (ACS_PRESENT | ACS_DSEG | ACS_WRITE)

#pragma pack (push, 1)  /* align structures to a byte boundary */

/* Segment desciptor definition */
typedef struct {
  word limit,
       base_l;
  byte base_m,
       access,
       attribs,
       base_h;
} DESCR_SEG;

/* GDTR register definition */
typedef struct {
  word limit;
  dword base;
} GDTR;

/* Interrupt desciptor definition */
typedef struct {
  word offset_l,
       selector;
  byte param_cnt,
       access;
  word offset_h;
} DESCR_INT;

/* IDTR register definition */
typedef struct {
  word limit;
  dword base;
} IDTR;

#pragma pack (pop)      /* align structures to default boundary */

#endif
