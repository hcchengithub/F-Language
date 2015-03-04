/*
**   forth.c
**
**   The Chern's FORTH interpreter
**
*/

#pragma     inline
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <dir.h>
#include    <io.h>
#include    <dos.h>
#include    "undocdos.h"
#include    "forth.h"

struct forthstack stack, auxstack;   /* F51  -hcchen5600 2008/12/24 21:31  */

FILE    *fp0;      /* The script file */
FILE    *fp;       /* The current input stream , can be NULL=SCRIPT or stdin or COMMANDLINE */
char    ss[LINESIZE+1];
int     debug;
struct  SBUF sbuf;   /* script buffer */
struct  MCACHE mcache[MCACHESIZE]; /* macro cahce look up table */
int     mcache_count;
int     cmdline_size;
char    **cmdline;
/* int     fromCL;  get next word from command line F50 drop it. Merged the logic into fp */
int     cryptogram;
int     cl_next;
BYTE    * Buffer = NULL;
union REGS iregs, oregs;
struct SREGS sregs;


int pop()
{   int temp;
    if (stack.tos) {
        temp = stack.data[stack.tos--];
    } else {
        printf ("\nPop empty stack! %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);
        stack.error = 1;
    }
    return temp;
}

void push(int data)
{
    if (++stack.tos != STACKSIZE) {
        stack.data[stack.tos] = data;
    } else {
        printf ("\nPush full stack! %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);
        stack.tos = 0; /* make it more fatal to avoid from proceed */
        stack.error = 1;
    }
}

int auxpop()
{   int temp;
    if (auxstack.tos) {
        temp = auxstack.data[auxstack.tos--];
    } else {
        printf ("\nPop empty aux stack! %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);
        auxstack.error = 1;
    }
    return temp;
}

void auxpush(int data)
{
    if (++auxstack.tos != STACKSIZE) {
        auxstack.data[auxstack.tos] = data;
    } else {
        printf ("\nPush full aux stack! %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);
        auxstack.tos = 0; /* make it more fatal to avoid from proceed */
        auxstack.error = 1;
    }
}

/* given two ASCIIZ string , return 0 if they are identical */
int str_compare(char s1[], char s2[]){
    int i;
    for (i=0; i < 80; i++) {
        s1[i] = s1[i] <= 'Z' && s1[i] >= 'A' ? s1[i] - 'A' + 'a' : s1[i] ;   /* lower case x */
        s2[i] = s2[i] <= 'Z' && s2[i] >= 'A' ? s2[i] - 'A' + 'a' : s2[i] ;   /* lower case x */
        if (s1[i] != s2[i]) return -1;
        if (!s1[i]) return 0; /* they are both Nul */
    }
    printf("String too long. %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);      /* F54 */
    return 2;
}

/*
** Process next word
*/
FLSC one_word (char *token) /* the token in global ss , F50 change to token  -hcchen5600 2008/12/20 08:38 */
{   FLSC flsc;

    /* get the address in map file for debug */
    /* asm Global entering_one_word          */
    /* asm entering_one_word:                */

    if ((flsc=dic_forth(token))!=FAIL) { return flsc; }
    /* if (dic_cmos()==OK)  { return OK; } */   /* these never been used so removed since F35 */

    /* if (dic_env(token)==OK)   { return OK; } */
    if ((flsc=dic_env(token))!=FAIL) { return flsc; }     /* F50 re-write -hcchen5600 2008/12/20 23:46 */

    /* if (dic_script(token)==OK){ return OK; } */
    if ((flsc=dic_script(token))!=FAIL) { return flsc; }     /* F50 re-write -hcchen5600 2008/12/20 23:46 */

    /* if (dic_PnP()==OK) { return OK; }     F45 removes Pnp, obsoleted. H.C. Chen 2008/07/05 16:04:38 PM */
    /* if (dic_assembly(token)==OK){ return OK; } */
    if ((flsc=dic_assembly(token))!=FAIL) { return flsc; }     /* F50 re-write -hcchen5600 2008/12/20 23:46 */

    /* F744 for studying Alexei Frounze's PMode tutorials */
    /* if (dic_frounze(token)==OK){ return OK; } */
    if ((flsc=dic_frounze(token))!=FAIL) { return flsc; }     /* F50 re-write -hcchen5600 2008/12/20 23:46 */

    /* F51 prints the unknown word directly. Can't be print by higher level because token may be different */
    if (flsc==FAIL) {
    	printf ("\n%s ? unknown %04x:%04x\n", token, _DS, &sbuf.b[sbuf.p]);   /* F54 */
        return UNKNOWN; /* Unknown input */  /* F54 changed from return FAIL to UNKNOWN */
    }
    return SYNTAX; /* I am not sure what happened in this case !! */
}

int main(int argc, char **argv)
{
    int i;
    FLSC flsc;
    long filelen;
    char main_token[WORDSIZE];  /* f50  -hcchen5600 2008/12/20 08:45 */

    cmdline = argv;
    cmdline_size = argc;
    cl_next = 2;  /* command line next word */
    /* PnPreal = MK_FP(_CS, (unsigned) NullFunction); h.c. Chen 2008/07/05 16:04:22 PM F45 Obsoleted */  /* PnP BIOS real mode entry default function */

    restart:
    debug = 0; /* debug controls delay, (F55 remove) echo controls echo */
    sbuf.l = 0;
    one_word("getregs");   /* F54 hcchen5600 2009/02/27 09:18  */
    if (argc>=2 && ((str_compare(argv[1], "nul")!=0) && (str_compare(argv[1], "-")!=0))) {   /* command line */
        fp0 = fopen(argv[1], "rb");
        if (fp0==NULL) {
            puts("Script file open error.");
            return 255;
        }
        mcache_count = 0; for (i=0;i<MCACHESIZE;i++) mcache[i].name[0] = '\0';
        for (i=0;i<LINESIZE;i++) ss[i]='\0';
        fnsplit(argv[1], &ss[15], &ss[15], ss, &ss[9]);
        strupr(ss);
        strupr(&ss[9]);
        cryptogram = ss[ 0] ^ ss[ 1] ^ ss[ 2] ^ ss[ 3] ^ ss[ 4] ^ ss[ 5] ^ ss[ 6] ^ ss[ 7] ^ ss[10] ;
        cryptogram = cryptogram==0 ? 0x69 : cryptogram;
        filelen = filelength(fileno(fp0));
        sbuf.l = (size_t) filelen;   /* small model */

        /* F45 RN05, to avoid ; before EOF problem. Append a space to the end of script buffer */
        /* F54 reserve 80 characters for eval(string), so total 81 more characters appanded */
        sbuf.b = (char *) malloc(sbuf.l + 81);     
        if (sbuf.b == NULL || (long) sbuf.l != filelen) {
            puts("Script file too big, Fxx is built in small mode.");
            return 255;
        } else {
            if (sbuf.l != read(fileno(fp0), sbuf.b, sbuf.l)) {
                perror("Abnormal terminated reading script file.");
                return 255;
            }
        }
        sbuf.b[sbuf.l++] = ' ' ; /* F45 RN05, to avoid ; before EOF problem. Append a space to the end of script buffer */

        /* [ ] ���F�o�� ##autoexec , F54 �M�߭n���@�� eval(" defined @#autoexec if ##autoexec endif ") �s function�C �H�K��b C �̭��ϥ� F macros.  
               �p���A�~�n�� F54 �b�ª� .f �̨S�� ##autoexec �ɡA�໴�����L�Ceval �t�X defined �`���j�γB���C
        */

        flsc = eval(" defined @#autoexec if ##autoexec endif "); /* F54 �e�᪺�Ů�n�������n */
        if (flsc != OK) {  
                printf("Something wrong in ##autoexec\n");  /* F54 */
                goto getout;
        }
    }
    if (argc<=2) {  /* console mode */
        puts(
          /* 0        1         2         3         4         5         6         7         8 */
          /* 12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
            "                                                       r/n log:http://docs.goog\n"
            "Chern's F55 interpreter (C) 1995..2009 by H.C. Chen    le.com/Doc?id=dgzzwq68_5\n"
            "Usage: F55 [[script-filename|-|nul] [word ...]]        59ggdrkrcf pw : aaaa1111\n"
            "No word specified, we're in debug mode.                id : heculas@bigfoot.com\n"
        );
        debug = 1; /* (d  .. d) �� debug �٬O���Ϊ� */
        fp = stdin;
    }
    if (argc>=3) {  /* macro given by command line */
        self = _psp; parent = 0;   /* constructors of FORTH */
        auxstack.tos = stack.tos = auxstack.error = stack.error =0; /* constructor of stack */
        sbuf.p = 0;
        for (;;) {
            fp = (FILE *) COMMANDLINE; /* input stream from command line */
            /* fromCL = 1; get next word from command line. combined fp and fromCL -hcchen5600 2008/12/21 09:41 */
            if (next_word(fp, main_token)!=OK) {  /* was NULL, now changed to fp  -hcchen5600 2008/12/21 09:42 */
                break;
            }
            switch (flsc=one_word(main_token)) {  /* OK SYNTAX RETURN UNKNOWN ENDIF BREAKLOOP LOOP CONTINUELOOP ELSEE ENDIF EOF */
                case OK: case RETURN:
                    if (stack.error || auxstack.error) break;  /* F54 stack error terminates the program */
                    continue;
                case EOF:
                    printf("\nEOF encountered. %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);  /* F54 */
                    break;
                case ENDIFF: case BREAKLOOP: case LOOP: case MINUSLOOP: case PLUSLOOP: case CONTINUELOOP: case ELSEE:
                case SYNTAX:
                    printf("\nSyntax error1 %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);  /* F54 */
                    break;
                case UNKNOWN: case ABORT:
                    /* F54 , actual unknown word is not main_token, should be printed in one_word(), too late here */
                    break;
                default :  /* FAIL and ???? */
                    printf("\nUncertain error1 %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);  /* F54 */
                    break;
            }
            break;
        }
    } else {
        /* console mode, input stream is from stdin, something need special handling for debug */
        self = _psp; parent = 0;   /* constructors of FORTH */
        auxstack.tos = stack.tos = auxstack.error = stack.error =0; /* constructor of stack */
        for (;;) {
            fp = stdin; /* input stream from script memory */
            /* fromCL = 0; F50 */
            switch (flsc=forth()) {
                case RETURN:
                    break;
                case EOF:
                    printf("\nEOF encountered %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);  /* F54 */
                    continue;  /* �X���� command line , script , stdin ���O��ݡA�o�O�Ӧn�D�N�I -hcchen5600 2008/12/19 12:53  */
                case ENDIFF: case BREAKLOOP: case LOOP: case PLUSLOOP: case MINUSLOOP: case CONTINUELOOP: case ELSEE:
                case SYNTAX:
                    printf("\nSyntax error2 %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);  /* F54 */
                    continue;
                case RELOAD:
                    argc = 2;
                    fclose(fp0);
                    free(sbuf.b);
                    goto restart;
                case UNKNOWN: case ABORT:  /* F54 */
                    /* F54 , actual unknown word is not main_token, should be printed in one_word(), too late here */
                    auxstack.tos = stack.tos = auxstack.error = stack.error =0; /* F54 constructor of stack */
                    continue;
                default :  /* FAIL and ???? */
                    printf("\nUncertain error2 %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);  /* F54 */
                    continue;
            }
            break;
        }
    }

    getout:   /* f54  improve ##autoexec error check */

    if (argc>=2) {
        /* no need to close because scriptfp == fp0 */
        fclose(fp0);
    }
    if (stack.tos) {
        return pop();
    } else {
        return 0;
    }
}

/* F55
   
   hcchen5600 2009/03/13 09:34
   In between b" and " , the b" length only avoids  b" to close with a wrong " position, next_word() can still return a 
   wrong word in the binary string. next_word() should skip s" ." b"  // \ �o�ǪF��C�ݨӡA�����n��o�@���F�誺 search �\
   �ධ���_�ӡAnext_word() �n�ΡAword �����]�n�ΡC ." ���@�k���ӻP�s�� s" type ²�Ƶ��X�_�ӡAF54 ���e���g�k�ӫj�j�F�C�o��
   ���� search �� function �N�s�� sbuf.p quote_end(token) �a, ���K��o�@�z string �������F�F���׭���b 255 bytes �����A
   �p���i�H�u�Τ@�� byte �����ץΡC�X���ɩ����d��p�]���u�I�C���ݭn�Τj�Ŷ��̡A�i�H�� sfind �� #macro �~���h�C
   // and \  return sbuf.p index to the end of line \n character. length no limit.
   s" and ." return sbuf.p index to null which was the " character.  return EOF(-1) means length over 256 bytes.
   b"  return sbuf.p index to the space after the " character.
   
   �� ==> �H�e�b next_word() �̳B�z ."  // ���@�k���S�����n�C�ڲq���ɬO�Q sbuf.p ���N�q�d�k��F�A�H�� next_word() �n�Ǧ^ token 
   �P�ɧ� sbuf.p point ��U�@�ӾA�����a�I�Cs" �O��Ӽg���A�H s" ���ҴN���D�F�A�u�n�b s" �̧� sbuf.p ���n�N�i�H�C��L��P�A
   one_word() �S�����n�޳o�ӤߡA�Ϧӷd�����C

   �H�e�O�諸�I next_word() ���� ." // \ b" s" �����o�� string ��ӦY�����ܡA�ӵ� string �����e�N�|�Q sfind defined next_word() 
   �H�Υ��Ӫ��Ѫ��D unexpectedly ���C�G������ next_word() ���� skip �� " ���C 
   1. [x] quote_end() ������ next_word() �̰���A�G�ݥ[�j token filtering �P�_���C <== done f55a hcchen5600 2009/03/13 16:03 
   2. ��F�o�� string words �W��ɡAsbuf.p �w�g���V " ���F�Astring words �n�Ψ쪺 string �Y�n����k�� next_word() �ǤU�h�C
      ==> [x] �X�R sbuf.me , �� next_word(fp,token) �Ǧ^ token �ɡAsbuf.me ���b�� token ���� white space �W�C <== done F55a

   quote_end() return values
      sbufp points to the space after the recent string.
      -1 == EOF
      -2 == illegal length (over 255 bytes limit or length equals to 0 or 0x78). <==== obsoleted.
      -3 == not a string word
   
   hcchen5600 2009/03/13 22:48 [ ] quote_end() �̭��ˬd string size ���n�C next_word() �� low level �F�A��B�|�Ψ�C#macro ; �H
   �~���F��A�]�|�Q next_word() ���˹L�{�̬ݨ�A�Q�P error ���X�z�C�n�P string words error ���H�A�����O string words ���{�������A
   �Ӥ��ӬO quote_end() �� next_word(). �{�b���F sbuf.me sbuf.p ���פw�M���, quote_end() ����ת��ˬd�ϦӬO respondent �C   
   �]�� quote_end() ���I preprocess �����D�Ab" ���{���ڥ��ݤ��� length == 0x78 ������, �s�Ĥ@�������Ӭݨ�C�ݨ�F�A�N�O�n�T��A
   �ӵ{���ۤv�N�i�H���A���� quote_end() �ܤ��۵M�a�h���C
*/   
unsigned quote_end (char * token, unsigned sbufp)  /* sbufp is supposed pointing at the space after the ending " or NUL */
{  unsigned i;
   
   if (!((token[0]=='.' && token[1]=='"' && token[2]=='\0') || 
         (token[0]=='s' && token[1]=='"' && token[2]=='\0') || 
         (token[0]=='S' && token[1]=='"' && token[2]=='\0') || 
         (token[0]=='b' && token[1]=='"' && token[2]=='\0') || 
         (token[0]=='B' && token[1]=='"' && token[2]=='\0') || 
         (token[0]=='\\' && token[1]=='\0'                ) ||
         (token[0]=='/' && token[1]=='/' && token[2]=='\0')
        )
      ) return -3;  /* -3 == not a string word */
   
   switch (token[0]){ /* �T�w�O�Y�� string word �~�|�Ө�o�̡A�u�O����@�U�Ӥw // \ ." s" b" */
   	case '.' :
   	case 's' : case 'S' :
                   for (; sbuf.b[sbufp]!='\0' && sbuf.b[sbufp]!='"'; sbufp++){  /* find the ending " or NULL */
                       if (sbufp>=sbuf.l) return EOF;
                   }
                   return sbufp+1;

   	case 'b' : case 'B' :    /* \" is allowed for b" strings, not allowed for ." and s" strings, same as forth */
                   i = (unsigned char) sbuf.b[sbufp + 1]; /* get the length */
                   if ( i == 'x') {  /* F55 'x'==0x78 means this is the first time */
                       for (i=0,sbufp+=2; ((sbuf.b[sbufp]!='"')||(sbuf.b[sbufp-1]=='\\'&&sbuf.b[sbufp]=='"')); i++,sbufp++) if (sbufp >= sbuf.l) return EOF;  /* +2 for space and x */
                       sbuf.b[sbufp-i-1] = i > 255 ? 0 : i; /* sbufp points at the space after the ending ", save the length */
                       return sbufp+1;  /* points to the space after " */
                   } else {
                       return sbufp+i+3;  /* sbufp points at the space after b", i is the length, +1 for the x length +2 for the ending " +3 to the space after " */
                   }

   	case '/' :
   	case '\\':
                   for (; sbuf.b[sbufp]!='\n' ; sbufp++){  /* find end of line */
                       if (sbufp>=sbuf.l) return EOF;
                   }
                   return sbufp;
   }
}

/*
** get next word to token
** it can be from CommandLine, stdin, or script buffer
** move sbuf.me to the ending white space after the found token.
** move sbuf.p to next point for next word searching 
** sbuf.me equals to sbuf.p usually but not string words (F55a)
*/
FLSC next_word(FILE *fp, char *token)
{   unsigned i,j;
    if (fp==(FILE *) COMMANDLINE) {
        /* fromCL = 0;  unified to use fp instead of fromCL  -hcchen5600 2008/12/21 09:44 */
        if (cl_next<cmdline_size) {
            strcpy(token, cmdline[cl_next++]);
            return OK;
        } else {
            return EOF;
        }
    } else if (fp==stdin) {
        if (fp->level <= 1) {
            printf("Ready: ");
        }
        if (fscanf(fp, "%36s", token)!=1) return EOF;   /* WORDSIZE=36 , read next word seperated by white spaces (not next line !) */

    } else if (fp==(FILE *)SCRIPT) {
        for (;;) {    /* skip white spaces and NUL */
            if (sbuf.p>=sbuf.l) return EOF;
            switch (sbuf.b[sbuf.p]) {
                case ' ': case '\t': case '\n': case '\r': case '\0':
                sbuf.p += 1;
                continue;
            }
            break;
        }
        for (i=0; i<WORDSIZE && (sbuf.p < sbuf.l); i++,sbuf.p++){  /* copy next word to token until reaching next white space , NUL, or WORDSIZE limit */
            if (sbuf.p>=sbuf.l) return EOF;                        /* �J�� WORDSIZE limit ���� cut ���A�o���oĵ�i�H ���o�A�����٤��T�w�o�� token �O�_�O�� word */
            switch (sbuf.b[sbuf.p]) {
                case ' ':case '\t':case '\n':case '\r':case '\0':
                    break;
                default:
                    token[i] = sbuf.b[sbuf.p];
                    continue;
            }
            token[i] = '\0';
            break;
        }
        sbuf.me = sbuf.p;    /* F55 �s�n������ token ��������m�A �ǳưw�� string words �� sbuf.p skip ����� " �᭱�h */
    	switch (i=quote_end(token, sbuf.p)) {
    	    case -1 : return EOF;
    	    case -3 : break;    /* none string word, do nothing */
    	    default :
    	    	sbuf.p = i;  /* F55 �w�� string words �� sbuf.p skip ����� " �h */
    	}
        
    }
    return OK;
}

/* gets() from script buffer
   [ ] f48 ���e�A�u�� 'call' ���O�|�Ψ�C ���Ӧn�n�X�i���з� forth �� 'evaluate' �� 'interpreter'�C
*/
int bgets(char *a_line, int max_len)
{   unsigned i;
    /* fp must be now from script buffer */
    for (;;) {
        if (sbuf.p>=sbuf.l) return EOF;
        switch (sbuf.b[sbuf.p]) {
            case ' ':case '\t':case '\n':case '\r':case '\0':
            sbuf.p += 1;
            continue;
        }
        break;
    }
    for (i=0; i<(max_len-1) && (sbuf.p < sbuf.l); i++,sbuf.p++){
        a_line[i] = sbuf.b[sbuf.p];
        switch (sbuf.b[sbuf.p]) {
            case '\n':case '\r':case '\0':
                break;
            default:
                continue;
        }
        a_line[++i] = '\0';
        break;
    }
    if (sbuf.p>=sbuf.l) return EOF;
    return OK;
}

/* forth() �u���� input �O sbuf.p , ���ӥι� sbuf.p �o�ˤ@�� global variable�C ���ӧ�W�s NEST(unsigned sbuf_ptr)�C
   ������ UNNEST �N�O ; �� ret �C�n�Q�@�Q�A�b���P input stream �����X�A NEST / UNNEST ���󤣦P�C -hcchen5600 2008/12/20 17:12 
*/
FLSC forth()    
{   FLSC flsc;
    char forth_token[WORDSIZE]; /* F50  -hcchen5600 2008/12/20 08:47 */

    for (;;) {
        if ((flsc = next_word(fp,forth_token))!=OK) return flsc;    /* get next word, if failed then handle the situation like an unexpected EOF happened */
        flsc=one_word(forth_token);  /* execute the word (the command). FLSC={OK SYNTAX FAIL RETURN UNKNOWN ENDIF BREAKLOOP LOOP CONTINUELOOP ELSEE ENDIF EOF} */
        /* debuging(); F49 drop this  -hcchen5600 2008/12/19 12:58 */
        switch (flsc){
                case OK:
                    if (stack.error || auxstack.error) return ABORT;  /* F54 stack error terminates the macro */
                    continue;
                case FAIL:
                case UNKNOWN:
                case SYNTAX:
                case RETURN:
                case ENDIFF:
                case BREAKLOOP:
                case LOOP:
                case PLUSLOOP:
                case MINUSLOOP:
                case CONTINUELOOP:
                case ELSEE:
                case EOF:
                default:
                        return flsc;
        }
    }
}
