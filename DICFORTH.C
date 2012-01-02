/*
**  dicforth.c
**
*/

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <conio.h>
#include    <dos.h>
#include    <bios.h>
#include    <alloc.h>
#include    "undocdos.h"
#include    "forth.h"

#if 0       /* was for debugging PMode ISR when studying Alexie's TUT05 */
void dic_deadloop(int errorcode)
{
     outport(0x80, errorcode);
     again:
     goto again;
}
#endif

/* F54 
   
   eval(" one line ")  字串前後的 space 好像有必要
   
   要從 C 裡去 call F macros, 首先要把 F interpreter 的 instruction pointer sbuf.b sbuf.p sbuf.l 暫時指到這裡來， fp 也要安排好。 
   把這一行 copy 到 sbuf.b[sbuf.l] 後面去。確定 sbuf.b 後面有保留 80 bytes 給 eval()。 然後請 forth() 來執行本地這一行 script.

*/   
FLSC eval (char * onelinescript)  /* Maximum 80 characters, 多的會「無預警地」被切掉（這種地方不想太苛求） */
{  struct SBUF sbuf_backup;
   FILE *fp_backup;
   FLSC flsc;
   int i;
   
   fp_backup=fp; sbuf_backup.p=sbuf.p; sbuf_backup.l=sbuf.l;  /* save */
   fp = (FILE *)SCRIPT; sbuf.p = sbuf.l ;
   for (i=0; (i<80) && onelinescript[i]; i+=1) {
       sbuf.b[sbuf.l+i] = onelinescript[i];
   }
   sbuf.l += i;
   
   flsc = forth();
   switch (flsc) {  /* F54 OK SYNTAX RETURN UNKNOWN ENDIF BREAKLOOP LOOP CONTINUELOOP ELSEE ENDIF EOF */
       case OK: case EOF: case RETURN:
       	   flsc = OK;
           break;
       default : 
           printf("Something wrong in eval(\"%s\") %04x:%04x flsc=%d\n", onelinescript, _DS, &sbuf.b[sbuf.p], flsc);  /* F54 */
   }
   fp=fp_backup; sbuf.p=sbuf_backup.p; sbuf.l=sbuf_backup.l; /* restore */
   return flsc;
}

void scan_decimal(char *token, int *i)
{  int x;
   *i = 0;
   for (x=0; x<5; x++) {  /* 65536 has only 5 digits */
      if (token[x]) {
           if (!(token[x]>='0' && token[x]<='9')) {
               /* dic_deadloop(0xabc1);  was for debugging PMode ISR when studying Alexie's TUT05 */
               printf("Fatal: incorrect decimal %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);   /* F54 */
               exit(127);
            }
           *i *= 10;
           *i += token[x] - '0';
      } else {
           return;
      }
   }
}

void scan_hex(char *token, int *i)
{   int x;
    *i = 0;
    for (x=0; x<4; x++) {  /* 4 char maximum 超過的給他去錯吧！ */
       if (token[x]) {
            if (!((token[x]>='0' && token[x]<='9') || (token[x]>='a' && token[x]<='f') || (token[x]>='A' && token[x]<='F'))) {
               /* dic_deadloop(0xabc2); was for debugging PMode ISR when studying Alexie's TUT05 */
               printf("Fatal: incorrect hexdecimal %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);      /* F54 */
               exit(127);
            }
            *i *= 16;
            if (token[x]>='0' && token[x]<='9'){
                *i += token[x] - '0';
            } else if (token[x]>='a' && token[x]<='f'){
                *i += token[x] - 'a' + 10;
            } else if (token[x]>='A' && token[x]<='F'){
                *i += token[x] - 'A' + 10;
            }
       } else {
             return;
       }
    }
}

FLSC ch_op_math(char *token)
{
    unsigned i,j,k;
    unsigned long l;
    unsigned long l1, l2, l3;   /* for testing L+ command */

    if (token[0]=='$') {
        scan_hex((char *)&token[1], (int *)&i);       /* 改寫, avoid using sscanf(), try to support PMode H.C. Chen 2008/07/13 PM 22:34:56  */
        push(i);
        return OK;
    }
    if (token[0]=='0') {
        scan_decimal((char *)&token[1], (int *)&i);   /* 改寫, avoid using sscanf(), try to support PMode H.C. Chen 2008/07/13 PM 22:34:56  */
        push(i);        
        return OK;
    }
    if (token[1]=='\0') {
        switch(token[0]) {
            case '+' :
                push(pop() + pop());
                return OK;
            case '*' :
                push(pop() * pop());
                return OK;
            case '/' :
                i = pop();
                j = pop();
                push(j/i);
                return OK;
            case '-' :
                push(0 - pop() + pop());
                return OK;
            case '&' :
                push(pop() & pop());
                return OK;
            case '~' :
                push(~pop());
                return OK;
            case '^' :
                push(pop() ^ pop());
                return OK;
        }
    }
    if ((str_compare(token,"|")==0)||(str_compare(token,"BR")==0)) {
        push(pop() | pop());
        return OK;
    }
    if ((str_compare(token,">")==0)||(str_compare(token,"GT")==0)) {
        push (pop()<pop());
        return OK;
    }
    if ((str_compare(token,"<")==0)||(str_compare(token,"LT")==0)) {
        push (pop()>pop());
        return OK;
    }
    if ((str_compare(token,">=")==0)||(str_compare(token,"GE")==0)) {
        push (pop()<=pop());
        return OK;
    }
    if ((str_compare(token,"<=")==0)||(str_compare(token,"LE")==0)) {
        push (pop()>=pop());
        return OK;
    }
    if ((str_compare(token,">>")==0)||(str_compare(token,"SR")==0)) {
        push ((unsigned)pop()>>1);
        return OK;
    }
    if ((str_compare(token,"<<")==0)||(str_compare(token,"SL")==0)) {
        push ((unsigned)pop()<<1);
        return OK;
    }

    if (str_compare(token,"==")==0) {
        push (pop()==pop());
        return OK;
    }
    if (str_compare(token,"!=")==0) {
        push (pop()!=pop());
        return OK;
    }
    if (str_compare(token,"OR")==0) {
        i = pop();
        j = pop();
        push(i || j);
        return OK;
    }
    if (str_compare(token,"AND")==0) {
        i = pop();
        j = pop();
        push(i && j);
        return OK;
    }
    if (str_compare(token,"NOT")==0) {
        push(!pop());
        return OK;
    }
    if (str_compare(token,"XOR")==0) {
        i = pop();
        j = pop();
        push(!i&&j || i&&!j);
        return OK;
    }
    if (token[0]=='\'') {
        if (!token[1]) push(' '); /* support space F473 H.C. Chen 2008-08-05 12:32:36 */
        for (i=1; token[i]; i++) {
            push(token[i]);
        }
        return OK;
    }
    if (str_compare(token,"L+")==0) {  /* F474 [a_high a_low b_high b_low -- a+b_high, a+b_low] */
        i = pop();     /* b_low  */
        j = pop();     /* b_high */
        l = (unsigned long) j * 65536L + i;
        i = pop();     /* a_low  */
        j = pop();     /* a_high */
        l += (unsigned long) j * 65536L + i;
        push((unsigned) (l >> 16));
        push((unsigned) l);
        return OK;
    }
    if (str_compare(token,"L-")==0) {  /* F474 [a_high a_low b_high b_low -- a-b_high, a-b_low] */
        i = pop();     /* b_low  */
        j = pop();     /* b_high */
        l = 0 - ((unsigned long) j * 65536L + i);
        i = pop();     /* a_low  */
        j = pop();     /* a_high */
        l += (unsigned long) j * 65536L + i;
        push((unsigned) (l >> 16));
        push((unsigned) l);
        return OK;
    }
    if (str_compare(token,"L*")==0) {  /* F474 [a_high a_low b-- a*b_high, a*b_low] */
        i = pop();     /* b      */
        j = pop();     /* a_low  */
        k = pop();     /* a_high */
        l = (unsigned long) k * 65536L + j;
        l *= i;
        push((unsigned) (l >> 16));
        push((unsigned) l);
        return OK;
    }
    if (str_compare(token,"L/")==0) {  /* F474 [a_high a_low b-- a/b_high, a/b_low] */
        i = pop();     /* b      */
        j = pop();     /* a_low  */
        k = pop();     /* a_high */
        l = (unsigned long) k * 65536L + j;
        l /= i;
        push((unsigned) (l >> 16));
        push((unsigned) l);
        return OK;
    }
    return FAIL;
}

/* if then else do loop s" ... etc must be in script mode -hcchen5600 2008/12/17 15:46  */
int inScriptMode(char *token)
{
    if (fp != (FILE *) SCRIPT) {  /* When not from script buffer. fp can be COMMANDLINE SCRIPT or stdin, fromCL can be 1 or 0 for command line  -hcchen5600 2008/12/17 15:35  */
        printf ("Error: %s must be in script mode.\n", token);
        return FALSE;
    } else {
        return TRUE; 
    }
}               

FLSC ch_conio(char *token)
{   int i;

    if ((token[0]=='\\'&& token[1]=='\0')||(token[0]=='/' && token[1]=='/'&& token[2]=='\0')) {
        if (! inScriptMode(token)) return ABORT;  /* skip if is not in script mode */
        /* sbuf.p = quote_end(token, sbuf.p); */
        return OK;
    }    
    if ((token[0]=='.' && token[1]=='"'&& token[2]=='\0')) {
        if (! inScriptMode(token)) return ABORT;  /* skip if is not in script mode */
    	sbuf.b[sbuf.p-1] = '\0';    /* change the ending " to NUL makes the string ASCIIZ for fputs() */ 
    	fputs(&sbuf.b[sbuf.me+1], stdout);  /* use fputs instead of puts to aovid the unexpected ending cr */
        return OK;
    }
    if ((token[0]=='b' || token[0]=='B' )&&(token[1]=='"'&& token[2]=='\0')) {    /* F55 [ -- addr ] allocate a binary space */
        if (! inScriptMode(token)) return ABORT;  /* skip if is not in script mode */
        push((unsigned) &sbuf.b[sbuf.me] + 2);  /* + 2 to skip the leading space after b" and the length byte x */
        if (sbuf.b[sbuf.me+1] == 'x' || (sbuf.p - sbuf.me) > (255+3) || (sbuf.p - sbuf.me) <= 3) {  /* the 3 = (length, " , space) */
            printf("Error: b\" length should not be 0, 0x78 or over 255 bytes.  %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);  /* F55 */
            return ABORT;
        }              
        return OK;
    }
    if ((token[0]=='s' || token[0]=='S')&&(token[1]=='"'&& token[2]=='\0')) {    /* [ -- addr ] allocate an ASCIIZ string */
        if (! inScriptMode(token)) return ABORT;  /* skip if is not in script mode */
        push((unsigned) &sbuf.b[sbuf.me] + 1);  /* + 1 to skip the leading space after s" */
        sbuf.b[sbuf.p-1] = '\0';    /* make the string NUL ended */
        return OK;
    }
    if (str_compare(token,"CR")==0) {
        putchar('\n');
        return OK;
    }
    if (str_compare(token,"CLS")==0) {
        clrscr();
        return OK;
    }
    if (str_compare(token,"STDIN")==0) {
        push((unsigned) stdin);
        return OK;
    }
    if (str_compare(token,"STDOUT")==0) {
        push((unsigned) stdout);
        return OK;
    }
    if (str_compare(token,"STDERR")==0) {
        push((unsigned) stderr);
        return OK;
    }
    if (str_compare(token,"STDAUX")==0) {
        push((unsigned) stdaux);
        return OK;
    }
    if (str_compare(token,"STDPRN")==0) {
        push((unsigned) stdprn);
        return OK;
    }
    if (str_compare(token,"BP")==0) {   /* F55 [ n -- ] break point number */
    	printf("\n ===== Breakpoint %d ===== \n", pop());
        fp = stdin;
        return OK;
    }
    if (str_compare(token,"SCRIPT")==0 || str_compare(token,"c")==0) {   /* F55 */
        if (sbuf.l) {     /* idiot-proof */
            fp = (FILE *) SCRIPT;
        }
        return OK;
    }
    if (str_compare(token,"SCRIPT_HERE")==0) {
        push ((unsigned)&sbuf.b[sbuf.p]);
        return OK;
    }
    if (str_compare(token,"TYPE")==0) {    /* 'type' and '*s' are similar but '*s' == 'type' + CR ; F52 hcchen5600 2009/02/17 12:54   */
        printf ("%s", (char *)pop());
        return OK;
    }
    if (str_compare(token,"*S")==0) {    /* 'type' and '*s' are similar but '*s' == 'type' + CR ; F52 hcchen5600 2009/02/17 12:54   */
        puts((char *)pop());
        return OK;
    }    
    if (str_compare(token,".")==0) {
        printf("%d", pop());
        return OK;
    }
    if (str_compare(token,".$")==0) {
        printf("%04X", pop());
        return OK;
    }
    if (str_compare(token,".!")==0){
        i=pop();
        sprintf((char *)i,"%6d",(char *)pop());
        push(i);
        *((char *)i+6) = ' ';
        return OK;
    }
    if (str_compare(token,".$!")==0){
        i=pop();
        sprintf((char *) i,"%6X",(char *) pop());
        push(i);
        *((char *)i+6) = ' ';
        return OK;
    }
    if (str_compare(token,"repeat")==0) {
        cl_next = stack.tos? pop()+1 : 2;
        return OK;
    }
    if (str_compare(token,"malloc")==0) {
        push((unsigned) malloc(pop()));
        return OK;
    }

    /* F45 pnp.c obseleted, "buffer" and "buffer!" moved from Pnp.c to here. H.C. Chen 2008/07/05 16:37:40 PM */
    if (str_compare(token,"Buffer") == 0) {
        /* [ -- offset] */
        push ((unsigned) Buffer);
        return OK;
    }
    if (str_compare(token,"Buffer!") == 0) {
        /* [offset --] */
        if (Buffer!=NULL) {
            /* dic_deadloop(0xabc3); was for debugging PMode ISR when studying Alexie's TUT05 */
            puts("Fatal: Buffer! overwrite");
            exit(1);
        }
        Buffer = (unsigned char *) pop();
        return OK;
    }
    if (str_compare(token,"reload") == 0) {
        return RELOAD;
    }
    return FAIL;
}

/* 
   F44, F47 and older versions have bug on the wait command.
   Thanks to WKS RD SW Tina Tang , Ivy Yue and Cindy Wang who have improved this wait() function. 
   Due to the 16 bits ticks limitation, this wait() can only be up to about 18 hours maximum.
   Refer to http://elearnwksrd.wistron.com.cn/xms/forum/show.php?id=1819 
*/
void wait(unsigned tick_count)
{
    unsigned long oldtick;
    unsigned long newtick;
    volatile unsigned long far *fpl;  /* far pointer to a long */
  
    fpl = MK_FP(0x40, 0x6c);  /* Low memory 0040:006c is DOS's time tick counter 32 bits */
    oldtick = *fpl;
    do{
       newtick=*fpl;
       if(newtick-oldtick!=0)
          {oldtick=newtick;
           tick_count--;}
      }while(tick_count!=0);
     return;
}

FLSC ch_debug(char *token)
{   int i;
    unsigned char far *fpc; /* far pointer to char */
    unsigned char *pc;      /* pointer to char */
    unsigned far *fpw;      /* far pointer to unsigned */
    unsigned *pw;           /* pointer to unsigned */

    if (token[1]=='\0') {
        switch(token[0]) {
            case 'i' :
            case 'I' :
                push (inportb(pop()));
                return OK;
            case 'o' :
            case 'O' :
                outportb(pop(),pop());
                return OK;
        }
    }
    if (str_compare(token,"iw")==0) {
        push (inport(pop()));
        return OK;
    }
    if (str_compare(token,"ow")==0) {
        outport(pop(),pop());
        return OK;
    }
    if (str_compare(token,"PEEK")==0||str_compare(token,"c@")==0) {   /* F50 fetch or read memory byte  -hcchen5600 2008/12/20 09:28 */
        pc = (unsigned char *) pop();
        push((int)*pc);
        return OK;
    }
    if (str_compare(token,"POKE")==0||str_compare(token,"c!")==0) {   /* F50 store or write memroty byte  -hcchen5600 2008/12/20 09:30 */
        i = pop();
        pc = (unsigned char *) pop();
        *pc = (unsigned char) i;
        return OK;
    }
    if (str_compare(token,"PEEKW")==0||str_compare(token,"@")==0) {  /* F50 fetch memory word  -hcchen5600 2008/12/20 09:31 */
        pw = (unsigned *) pop();
        push(*pw);
        return OK;
    }
    if (str_compare(token,"POKEW")==0||str_compare(token,"!")==0) {  /* F50 store word  -hcchen5600 2008/12/20 09:32 */
        i = pop();
        pw = (unsigned *) pop();
        *pw = (unsigned) i;
        return OK;
    }
    if (str_compare(token,"FPEEK")==0) {               /* far 的，就不再給 alias 了, fc@ 很難看 -hcchen5600 2008/12/20 09:37 */
        fpc = (unsigned char far *)((unsigned)pop() + ((long)pop() << 16));
        push((int) *fpc);
        return OK;
    }
    if (str_compare(token,"FPOKE")==0) {
        i = pop();
        fpc = (unsigned char far *)((unsigned)pop() + ((long) pop() << 16));
        *fpc = (unsigned char) i;
        return OK;
    }
    if (str_compare(token,"FPEEKW")==0) {
        fpw = (unsigned far *)((unsigned)pop() + ((long)pop() << 16));
        push((int) *fpw);
        return OK;
    }
    if (str_compare(token,"FPOKEW")==0) {
        i = pop();
        fpw = (unsigned far *)((unsigned)pop() + ((long) pop() << 16));
        *fpw = (unsigned) i;
        return OK;
    }
    if (str_compare(token,"WAIT")==0) {
        wait(pop());
        return OK;
    }
    if (str_compare(token,"DEBUG")==0) {
        debug = pop();
        return OK;
    }
    /* if (str_compare(token,"ECHO")==0) {
        echo = pop();
        return OK;
    } 
    if (str_compare(token,"&ECHO")==0) {
        push ((int)&echo);
        return OK;
    }
    */
    if (str_compare(token,"&DEBUG")==0) {
        push ((int)&debug);
        return OK;
    }
    if (str_compare(token,"&SS")==0) {
        push ((int)&ss);
        return OK;
    }
    if (str_compare(token,"&tos")==0) {
        /* [... a b -- ... a b &b] where &a = &tos - 2 */
        push((int)stack.data+2*stack.tos);
        return OK;
    }
    if (str_compare(token,"tos")==0) {
        /* [ -- n] number of stack entry */
        push(stack.tos);
        return OK;
    }
    if (str_compare(token,"IREGS")==0) {
        push ((int)&iregs);
        return OK;
    }
    if (str_compare(token,"OREGS")==0) {
        push ((int)&oregs);
        return OK;
    }
    if (str_compare(token,"SREGS")==0) {
        push ((int)&sregs);
        return OK;
    }
    if (str_compare(token,"INT86")==0) {
        int86(pop(), &iregs, &oregs);
        return OK;
    }
    if (str_compare(token,"INT86X")==0) {
        int86x(pop(), &iregs, &oregs, &sregs);
        return OK;
    }
    return FAIL;  /* Unknown */
}

FLSC ch_call(char *token)
{   int i, j, fortherr=FAIL;
    FILE *tempfp=NULL;
    unsigned old_position;

    if (str_compare(token,"SFIND")==0||str_compare(token,"DEFINED")==0) {  /* sfind and defined [-- addr] Found address or NULL */
        if (next_word(fp, ss)!=OK) {
            printf("SFIND: find what? %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);      /* F54 */
            return SYNTAX;
        } else {       
            /* next word 到手， 此時 &ss[1] 存有要找的 token */
            
            tempfp = fp; fp = (FILE *)SCRIPT; old_position = sbuf.p; sbuf.p = 0;  /* 準備從頭尋找 the given token */

            /* search mcache first */
            for (i=0; i<mcache_count; i++) {
                if (str_compare(&ss[1],mcache[i].name)==0) { /* F54 ss[0] 要求 user 加上 '_' 以避免下令時的 target string 與真正的 target string 混淆。這裡要剔除之 */
                    sbuf.p = mcache[i].position;
                    goto sfind_found;
                }
            }

            /* search the entire script buffer */
            do {
                if (next_word(fp, &ss[40])!=OK) {  /* 利用 ss[40] 當暫存區，因為 token 的上限是 36 characters */
                    /* End of script bufer reached */
                    if (!(token[0]=='d'||token[0]=='D')) {   
                        printf("SFIND: Can't find '%s'(%04x:%04x)\n", &ss[1], _DS, &sbuf.b[sbuf.p] );
                        return UNKNOWN;
                    } else {          
                    	push(0);  /* 'defined' return 0 when not found */
                        return OK;
                    }
                }
            } while (str_compare(&ss[1], &ss[40])!=0);  /* 看找到了嗎？ &ss[1] is target ss[40] is the temp */

            /* add new found sfind item into mcache */
            if (mcache_count < MCACHESIZE) {
                j = 0;
                do {
                    mcache[mcache_count].name[j] =ss[1+j]; /* F54 ss[0] 要剔除 */
                } while(ss[1+j++]);
                mcache[mcache_count].position = sbuf.p;
                mcache_count += 1;
            } else {
                puts("Warning: Macro cache buffer full\007");
            }

            sfind_found:
            push((unsigned)&sbuf.b[sbuf.p] + 1);   /* return the found sfind address. F474 + 1, 當變數用時可以直接用而不會破壞該 space */
            sbuf.p = old_position; fp = tempfp;
        }
        return OK;
    }
    if (str_compare(token,"ClearCache")==0) {
        mcache_count = 0;
        return OK;
    }
    if (str_compare(token,".MCACHE")==0) {
        for (i=0; i<mcache_count; i++) {
            printf("#%2d %5d %s\n",
                i,
                mcache[i].position,
                mcache[i].name
            );
            do {
                if (bioskey(1)) {
                    if(bioskey(0)==0x011b) break;
                }
            } while (bioskey(2) & 0x0f);
        }
        return OK;
    }
    if (token[0]=='#' && token[1]=='#' && sbuf.l) {
        tempfp = fp; fp = (FILE *) SCRIPT; old_position = sbuf.p; sbuf.p = 0;  /* 準備從頭找起 */
        for (i=0; i<mcache_count; i++) {                         /* 先看看 cache 裡有沒有 */
            if (str_compare(&token[1],mcache[i].name)==0) {
                sbuf.p = mcache[i].position;
                goto macro_found;
            }
        }
        do {   /* cache 裡沒有，真的要從頭找起了 */
            if (next_word(fp, &ss[40])!=OK) {   /* search the entire script file for the macro, use ss[40] as the scratch buffer */
                /* End of script bufer reached */
                sbuf.p = old_position;
                fp = tempfp;
                return FAIL;  /* Macro not found */
            }
        } while (str_compare(&token[1],&ss[40])!=0);   /* ss[40] 含 # */

        /* 找到了才會來到這裡，此時 sbuf.p 指在該 macro 的下一個位址上，忙了半天就是為了這個。 */
        if (mcache_count < MCACHESIZE) {        /* add into the cache list */
            strcpy(mcache[mcache_count].name,&token[1]);
            mcache[mcache_count].position = sbuf.p;
            mcache_count += 1;
        } else {
            puts("Warning: Macro cache buffer full\007");
        }

        macro_found:
        fortherr = forth();    /* execute the found macro */
        sbuf.p = old_position;
        fp = tempfp;
        return (fortherr==RETURN) ? OK : fortherr;   /* F54 */
    }
    return FAIL;    /* F54 */
}

FLSC dic_forth(char *token)
{   int err;
    if ((err=ch_call(token))!=FAIL) return err;  /* OK or EOF or SYNTAX */
    if ((err=ch_forth(token))!=FAIL) return err; /* OK or EOF or SYNTAX */
    if ((err=ch_op_math(token))!=FAIL) return err;
    if ((err=ch_debug(token))!=FAIL) return err;
    if ((err=ch_conio(token))!=FAIL) return err;
    return FAIL; /* FAIL == Unknown */
}
