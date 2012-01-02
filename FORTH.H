/*
**  forth.h
**
*/

#if 0  /* old method of below #defines have now been replaced by enum FLSC , F Language Syntax Cases FLSC  -hcchen5600 2008/12/26 08:35  */
    // #define     OK                   0
    // #define     SYNTAX              10
    // #define     FAIL                20    /* FAIL 表示：「dic_xxx() ch_xxx() 裡頭找不到該 word, 請讓別人試試看」。 只用在 one_word() 之內，離開 one_word() 此 case 改傳回 UNKNOWN */
    // #define     RETURN              30    /* ; and ret */
    // #define     UNKNOWN             40    /* one_word() 確定該 word 找不到時 */
    // #define     ENDIFF              50
    // #define     BREAKLOOP           60    /* break ?break */
    // #define     LOOP                70
    // #define     PLUSLOOP            71
    // #define     MINUSLOOP           72
    // #define     CONTINUELOOP        80    /* continue  */
    // #define     ELSEE               90    /* else  F50  -hcchen5600 2008/12/20 18:31 */
    // #define     RELOAD             100    /* F50  -hcchen5600 2008/12/21 08:21 */
    // #define     ABORT              110    /* F54 stack error, */
#endif
        
typedef enum {
            OK,
            SYNTAX,
            FAIL,              /* FAIL 表示：「dic_xxx() ch_xxx() 裡頭找不到該 word, 請讓別人試試看」。 只用在 one_word() 之內，離開 one_word() 此 case 改傳回 UNKNOWN */
            RETURN,            /* ; and ret */
            UNKNOWN,           /* one_word() 確定該 word 找不到時 */
            ENDIFF,
            BREAKLOOP,         /* break ?break */
            LOOP,
            PLUSLOOP,
            MINUSLOOP,
            CONTINUELOOP,      /* continue  */
            ELSEE,             /* else  F50  -hcchen5600 2008/12/20 18:31 */
            RELOAD,            /* F50  -hcchen5600 2008/12/21 08:21 */
            ABORT              /* F54 */
         /* EOF = -1           defined in stdio.h , any problem?  */
        } FLSC;

#define     SCRIPT            0x11    /* fp the current input stream is from script buffer  -hcchen5600 2008/12/21 09:07 */
#define     COMMANDLINE       0x22    /* 這個值要小一點以防跟真的 FILE *fp 撞上 */

#define     STACKSIZE           100
#define     WORDSIZE            36    /* f50 以前沒有定 -hcchen5600 2008/12/20 08:44  */
#define     LINESIZE            400   /* spawn needs 128 bytes after ss[200]  -hcchen5600 2008/12/08 18:49 */
#define     MCACHESIZE          100

struct forthstack {
    int tos; /* top of stack */
    int data[STACKSIZE];
    int error;  /* F54 improve error handling */
};

struct SBUF {  /* script file  buffer */
    char *b;   /* script buffer */
    unsigned p; /* script index, where to start next word searching, major used by next_word() */
    unsigned me; /* F55 script index, points to end of recent token, given by next_word() */
    unsigned l; /* script length */
};

extern struct SBUF sbuf;
extern char ss[LINESIZE+1];
extern BYTE * Buffer;

struct MCACHE {
    char name[40];  /* WORD max length 39 characters */
    unsigned position;
};
extern struct MCACHE mcache[MCACHESIZE];
extern int mcache_count;

struct BCOMMA {  /* compiler command b, for assembly support needs this structure */
    unsigned here;    /* program counter */
    unsigned offset;  /* b, buffer offset. Note! b, buffer is a NUL ended buffer. For boundary check. */
    unsigned segment; /* b, buffer segment */
};


#if 0
extern FILE *cmosfp;    /* the CMOS script file */
  CMOS script file and F language macro script file was the same file.
  Because F language was found to operate MS24 (MSY0) cmos.def.
  Since F35, I remove this feature for it is never used.
#endif

extern union REGS iregs, oregs;
extern struct SREGS sregs;
extern FILE *scriptfp;  /* the script file */
extern FILE *fp;        /* the input stream */
extern struct forthstack stack, auxstack;
extern WORD parent, self;
extern int  debug, echo;
extern int  cryptogram;  /* forth.c */
extern int  cl_next;  /* command line next word */
/* extern int  (far * PnPreal)();  F45 Obsoleted H.C. Chen 2008/07/05 16:03:45 PM */

void push(int data);
int pop(void);

int auxpop(void);
void auxpush(int data);

FLSC forth(void);
FLSC ch_forth(char *token);
FLSC dic_forth(char *token);  /* F50  -hcchen5600 2008/12/21 06:28 */
FLSC dic_env(char *token);
/* int dic_cmos(void); */
FLSC dic_script(char *token);
/* int dic_PnP(void);  F45 Obsoleted H.C. Chen 2008/07/05 16:03:53 PM */
FLSC next_word(FILE *, char *);
void wait(unsigned tick_count);
int bgets(char *ss, int max_len);
void NullFunction(void);
FLSC dic_assembly(char *token);   /* F45 H.C. Chen 2008/07/07 08:27:08 AM, inline assembly bits put all together */

/* F47 H.C. Chen 2008/07/11 17:30:36 PM , PMode avoid touching segment register */
int str_compare(char *s1, char *s2);
void scan_hex(char *ss, int *i);
void scan_decimal(char *ss, int *i);
FLSC dic_frounze(char *token);
int inScriptMode(char *token);  /* in dicforth.c  -hcchen5600 2008/12/17 17:37 */
FLSC eval (char *onelinescript); /* F54 */

