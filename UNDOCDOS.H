/* UNDOCDOS.H - typedefs for Undocumented DOS Second Edition programs
 *      Jim Kyle - July 1992
 */

#ifndef _UNDOCDOS_
#define _UNDOCDOS_

#pragma pack(1)                 /* for MSC                      */

typedef enum { FALSE, TRUE } BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long ULONG;
typedef unsigned long DWORD;
typedef void far *FP;
typedef FP far *FPP;

#ifndef MK_FP
#define MK_FP(seg,ofs)  ((FP)(((ULONG)(seg) << 16) | (ofs)))
#endif

#ifdef __TURBOC__
#define ASM asm
#else
#define ASM _asm
#endif

typedef struct DDVtag {         /* Device Driver header         */
    struct DDVtag far * next;
    unsigned attr;
    unsigned stratofs;
    unsigned introfs;
    union {
        BYTE name[8];
        BYTE blk_cnt;
        } u;
    } DDVR, far * LPDDVR;

/* some device attribute bits */
#define CHAR_DEV    0x8000
#define NOT_IBM     0x2000
#define HAS_INT29   0x0010
#define IS_CLOCK    0x0008
#define IS_NUL      0x0004
#define IS_STDOUT   0x0002
#define IS_STDIN    0x0001

typedef struct {                /* device driver command packet */
    BYTE hdrlen;
    BYTE unit;
    BYTE command;               /* 0 to initialize driver       */
    WORD status;                /* 0x8000 is error              */
    BYTE reserv[8];
    BYTE nunits;
    WORD brkofs;                /* break adr on return          */
    WORD brkseg;                /* break seg on return          */
    WORD inpofs;                /* SI on input                  */
    WORD inpseg;                /* _psp on input                */
    BYTE NextDrv;               /* next available drive         */
  } DDCMDPKT;

#define DD_INIT 0

typedef struct {                /* Memory Control Block entry   */
    BYTE type;                  /* 'M'=in chain; 'Z'=at end     */
    WORD owner;                 /* PSP of the owner             */
    WORD size;                  /* in 16-byte paragraphs        */
    BYTE unused[3];
    BYTE dos4[8];               /* filename, if DOS4+           */
    } MCB, far * LPMCB;

typedef struct DPBtag {         /* DOS Parameter Block entry    */
    BYTE drive;
    BYTE unit;
    WORD bytes_per_sect;
    BYTE sectors_per_cluster;
    BYTE shift;
    WORD boot_sectors;
    BYTE copies_fat;
    WORD max_root_dir;
    WORD first_data_sector;
    WORD highest_cluster;
    union {                     /* version-dependent areas      */
        struct {
            BYTE sectors_per_fat;
            WORD first_dir_sector;
            LPDDVR driver;
            BYTE media_descriptor;
            BYTE access_flag;
            struct DPBtag far * next;
            WORD curcluster;
            BYTE curdirpath[64];
            } v2;
        struct {
            BYTE sectors_per_fat;
            WORD first_dir_sector;
            LPDDVR driver;
            BYTE media_descriptor;
            BYTE access_flag;
            struct DPBtag far * next;
            WORD freeptr;
            WORD nfree;
            } v3;
        struct {
            WORD sectors_per_fat;
            WORD first_dir_sector;
            LPDDVR driver;
            BYTE media_descriptor;
            BYTE access_flag;
            struct DPBtag far * next;
            WORD freeptr;
            WORD nfree;
            } v45;
        } ver;
    } DPB, far * LPDPB;

typedef struct SFTtag {         /* System File Table entry      */
    union {
        struct {
            BYTE nusers;        /* number of handles using this */
            BYTE openmode;
            BYTE fileattr;
            BYTE drive;
            BYTE name[11];
            WORD unk1;
            WORD unk2;
            ULONG size;
            WORD fdate;
            WORD ftime;
            BYTE devattr;
            union {
                LPDDVR devdrvr;
                struct {
                    WORD first_cluster;
                    WORD rel_cluster;
                    } file;
                } type;
            WORD abs_cluster;
            WORD unk3;
            ULONG fileposn;
            } v2;
        struct {
            WORD nusers;        /* number of handles using this */
            WORD openmode;
            BYTE fileattr;
            WORD devinfo;
            union {
                LPDDVR devdrvr;
                LPDPB  dpb;
                } type;
            WORD first_cluster;
            WORD ftime;
            WORD fdate;
            ULONG size;
            ULONG fileposn;
            WORD rel_cluster;
            WORD abs_cluster;
            WORD dir_sector;
            BYTE dir_entry;
            BYTE name[11];
            struct SFTtag far * prev;
            WORD machine_number;/* this and prev used by SHARE  */
            WORD owner;         /* PSP segment of owner         */
            WORD share_rec;     /* offset in SHARE of record    */
            } v3;
        struct {
            WORD nusers;        /* number of handles using this */
            WORD openmode;
            BYTE fileattr;
            WORD devinfo;
            union {
                LPDDVR devdrvr;
                LPDPB  dpb;
                } type;
            WORD first_cluster;
            WORD ftime;
            WORD fdate;
            ULONG size;
            ULONG fileposn;
            union {
                struct {
                    WORD rel_cluster;
                    ULONG dir_sector;
                    BYTE dir_entry;
                    } loc;
                struct {
                    FP redirifs;
                    BYTE unk4[3];
                    } net;
                } t;
            BYTE name[11];
            struct SFTtag far * prev;
            WORD machine_number;/* this and prev used by SHARE  */
            WORD owner;         /* PSP segment of owner         */
            WORD share_rec;     /* offset in SHARE of record    */
            WORD abs_cluster;
            FP ifsdriver;       /* NULL if native DOS           */
            } v45;
        } ver;
    } SFT, far * LPSFT;

typedef struct SFTBtag {        /* System File Table block      */
    struct SFTBtag far * next;
    WORD here;
    SFT first;
    } SFTB, far * LPSFTB;

typedef struct {                /* Current Directory Structure  */
    BYTE path[67];
    WORD flags;
    LPDPB  dpb;                 /* different in DR-DOS?         */
    union {
        struct {
            WORD start_cluster;
            ULONG ffff;
            } loc;
        struct {
            WORD what;
            FP what2;
            } net;
        } in;
    WORD slash_offset;
    } CDS, far * LPCDS;

/* some CDS flag values */
#define CDS_INV     0
#define CDS_IFS     0xC000
#define CDS_NET     0x8000
#define CDS_PHYS    0x4000
#define CDS_JOIN    0x2000
#define CDS_SUBST   0x1000

typedef struct {                /* List of Lists                */
    LPDPB  dpb;                 /* first DPB                    */
    LPSFTB sft;                 /* first SFT block              */
    LPDDVR clock;               /* current clock driver         */
    LPDDVR con;                 /* current CON driver           */
    union {                     /* version-dependent areas      */
        struct {
            BYTE lastdrv;       /* number of drives             */
            WORD secsiz;
            FP disk_buff;
            DDVR nul;   /* not a pointer */
            } v2;
        struct {
            BYTE blk_dev;
            WORD secsiz;
            FP disk_buff;
            BYTE lastdrv;
            FP WorkArea;
            WORD WkAreaSz;
            LPSFTB fcb;
            WORD prot_fcb;
            DDVR nul;   /* not a pointer */
            } v30;
        struct {
            WORD secsiz;
            FP disk_buff;
            LPCDS cds;
            LPSFTB fcb;
            WORD prot_fcb;
            BYTE blk_dev;
            BYTE lastdrv;
            DDVR nul;   /* not a pointer */
            BYTE join;
            } v31up;
        } ver;
    } LOL, far * LPLOL;

#define MCB_FM_SEG(seg)     ((seg) - 1)
#define IS_PSP(mcb)         ((FP_SEG(mcb) + 1 == (mcb)->owner) && \
                 (*(WORD far *) MK_FP(FP_SEG(mcb) + 1, 0) == 0x20CD))

#define ENV_FM_PSP(psp_seg) (*((WORD far *) MK_FP(psp_seg, 0x2c)))
#define PARENT(psp_seg)     (*((WORD far *) MK_FP(psp_seg, 0x16)))

typedef struct {                /* Program Segment Prefix       */
    WORD sig;                   /* 0000 always CD 20 (INT 20)   */
    WORD Nxtgraf;               /* 0002 first unused segment    */
    BYTE Skip1;                 /* 0004 filler to align next    */
    BYTE CPMCall[5];            /* 0005 CP/M-like service call  */
    FP   ISV22;                 /* 000A documented ISR vectors  */
    FP   ISV23;                 /* 000E " (saved at start)      */
    FP   ISV24;                 /* 0012 "                       */
    WORD Parent_ID;             /* 0016 PSP of parent process   */
    BYTE HTable[20];            /* 0018 indices into SFT        */
    WORD EnvSeg;                /* 002C environment segment     */
    FP   SavStk;                /* 002E saved SS:SP at INT21    */
    WORD Nhdls;                 /* 0032 nbr of handles avail    */
    FP   HTblPtr;               /* 0034 ptr to handle table     */
    FP   ShareChn;              /* 0038 SHARE's closing chain   */
    BYTE Skip2;                 /* 003C unknown                 */
    BYTE TruNamFlg;             /* 003D APPEND's TrueName flag  */
    BYTE Skip3[10];             /* 003E unknown                 */
    BYTE WOldApp;               /* 0048 Windows OldApp Flag     */
    BYTE Skip4[7];              /* 0049 unknown                 */
    BYTE Disp[3];               /* 0050 Unix-like dispatcher    */
    BYTE Skip5[2];              /* 0053 unknown                 */
    BYTE ExtFCB[7];             /* 0055 extended FCB1 area      */
    BYTE FCB1[16];              /* 005C documented FCB areas    */
    BYTE FCB2[20];              /* 006C        "                */
    BYTE Tailc;                 /* 0080 "command tail" count    */
    BYTE Tail[127];             /* 0081 start actual data here  */
    } PSP, far * LPPSP;

typedef struct {                /* EXE Program Header           */
    WORD sig;                   /* always 0x5A4D, 'MZ'          */
    WORD LeftOver;              /* No. of bytes on last page    */
    WORD Pages;                 /* No. of 512 byte pages        */
    WORD Items;                 /* No. of RelocationTableItems  */
    WORD HdrSiz;                /* in paragraphs                */
    WORD ReqSiz;                /* in paragraphs                */
    WORD DesSiz;                /* in paragraphs                */
    WORD InitSS;                /* in relative paragraphs       */
    WORD InitSP;                /* at entry                     */
    WORD ChkSum;
    WORD InitIP;                /* at entry                     */
    WORD InitCS;                /* in paragraphs                */
    WORD FirstReloItem;         /* Offset from beginning        */
    WORD OlayNbr;
    WORD Reserved[ 16 ];
    ULONG NewExe;               /* offset from front of file    */
    } EXE, far * LPEXE;

typedef struct {                /* New EXE Program Header       */
    WORD sig;                   /* always 0x454E, 'NE'          */
    char LnkVer;                /* Version number of linker     */
    char LnkRev;                /* Revision number of linker    */
    WORD EntryTable;            /* Offset to Entry Table        */
    WORD EntryTblSz;            /* Nbr of bytes in Entry Table  */
    long Chksum;                /* 32 bit checksum for file     */
    WORD Flags;                 /* Flag word                    */
    WORD AutoDataSeg;           /* Seg number for auto data seg */
    WORD InitHeap;              /* Initial heap alloc; 0 = no heap      */
    WORD InitStack;             /* Initial stack alloc; 0 in libraries  */
    WORD InitIP;                /* Initial IP setting                   */
    WORD InitCS;                /* Initial CS segment number            */
    WORD InitSP;                /* Initial SP setting                   */
    WORD InitSS;                /* Initial SS segment number            */
    WORD SegEntries;            /* Count of segment table entries       */
    WORD MRTEntries;            /* Entries in Module Reference Table    */
    WORD NonResSiz;             /* Size of non-res name table (bytes)   */
    WORD OffsetSegTbl;          /* Offset of Segment Table              */
    WORD OffsetResTbl;          /* Offset of Resource Table             */
    WORD NamOffset;             /* Offset of resident name table        */
    WORD MRTOffset;             /* Offset of Module Reference Table     */
    WORD ImportOffset;          /* Offset of Imported Names Table       */
    long NonResOffset;          /* FilPos of Non-resident Names Table   */
    WORD MoveableCount;         /* Nbr of movable entries               */
    WORD Align;                 /* Segment alignment shift count        */
    WORD ResSegs;               /* Count of resource segments           */
    BYTE ExeType;               /* Operating System flags               */
    BYTE XtraFlags;             /* Additional exe flags                 */
    WORD FastLdOffset;          /* offset to FastLoad area              */
    WORD FastLdSize;            /* length of FastLoad area              */
    WORD unknown1;
    WORD ExpVers;               /* Expected Windows version number      */
    } NEXE, far * LPNEXE;

#pragma pack()                 /* for MSC                      */

#endif /* _UNDOCDOS_ */
