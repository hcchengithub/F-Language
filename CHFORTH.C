/*
**  chforth.c
**
*/

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <process.h>
#include    <dos.h>
#include    <io.h>
#include    <conio.h>
#include    <bios.h>
#include    <alloc.h>
#include    "undocdos.h"
#include    "forth.h"

FLSC pass_if(void);
FLSC pass_else(void);
FLSC If(void);
FLSC Else(void);

/* label list from map file  -hcchen5600 2008/12/19 16:36  */
unsigned label_list[] = {
   (unsigned) printf     , /* 00 */
   (unsigned) fprintf    , /* 01 */
   (unsigned) sprintf    , /* 02 */
   (unsigned) vsprintf   , /* 03 */
   (unsigned) puts       , /* 04 */
   (unsigned) fputs      , /* 05 */
   (unsigned) fputc      , /* 06 */
   (unsigned) fputchar   , /* 07 */
   (unsigned) scanf      , /* 08 */
   (unsigned) fscanf     , /* 09 */
   (unsigned) sscanf     , /* 10 */
   (unsigned) vsscanf    , /* 11 */
   (unsigned) fflush     , /* 12 */
   (unsigned) gets       , /* 13 */
   (unsigned) fgets      , /* 14 */
   (unsigned) bioskey    , /* 15 */
   (unsigned) fgetc      , /* 16 */
   (unsigned) fgetchar   , /* 17 */
   (unsigned) wherex     , /* 18 */
   (unsigned) wherey     , /* 19 */
   (unsigned) fopen      , /* 20 */
   (unsigned) fclose     , /* 21 */
   (unsigned) open       , /* 22 */
   (unsigned) read       , /* 23 */
   (unsigned) write      , /* 24 */
   (unsigned) close      , /* 25 */
   (unsigned) eof        , /* 26 */
   (unsigned) stpcpy     , /* 27 */
   (unsigned) strncpy    , /* 28 */
   (unsigned) strupr     , /* 29 */
   (unsigned) strcat     , /* 30 */
   (unsigned) strlen     , /* 31 */
   (unsigned) strcpy     , /* 32 */
   (unsigned) memcpy     , /* 33 */
   (unsigned) malloc     , /* 34 */
   (unsigned) itoa       , /* 35 */
   (unsigned) ltoa       , /* 36 */
   (unsigned) atoi       , /* 37 */
   (unsigned) atol       , /* 38 */
   (unsigned) ultoa      , /* 39 */
   (unsigned) atexit     , /* 40 */
   (unsigned) abort      , /* 41 */
   (unsigned) freopen    , /* 42 */
   (unsigned) fdopen     , /* 43 */
   (unsigned) fseek      , /* 44 */
   (unsigned) ftell      , /* 45 */
   (unsigned) ioctl      , /* 46 */
   (unsigned) isatty     , /* 47 */
   (unsigned) setvbuf    , /* 48 */
   (unsigned) access     , /* 49 */
   (unsigned) lseek      , /* 50 */
   (unsigned) tmpnam     , /* 51 */
   (unsigned) unlink     , /* 52 */
   (unsigned) free       , /* 53 */
   (unsigned) filelength , /* 54 */
   (unsigned) getvect    , /* 55 */
   (unsigned) setvect    , /* 56 */
   (unsigned) perror     , /* 57 */
   (unsigned) tell       , /* 58 */
   (unsigned) ungetc     , /* 59 */
   (unsigned) farmalloc  , /* 60 */
   (unsigned) farfree    , /* 61 */
   (unsigned) farcoreleft  /* 62 */
};


#if 0    /* was for debugging PMode ISR when studying Alexie's TUT05 */
void ch_deadloop(int errorcode)
{
     outport(0x80, errorcode);
     again:
     goto again;
}
#endif

FLSC If (void)
{   FLSC flsc;

    /* If 到 Else/Endif 中間以 NEST(Forth()) 伺候,故 NEST(forth()) 的結束就有很多種
       了: ';', 'ret', 'else','endif','loop','', '','' ... etc 分別要有對應的傳回值
       如此一來，NEST(forth()) 的結束應該用 case 來處理     -hcchen5600 2008/12/20 18:05
    */
    flsc = forth();    /* for easier debug  OK SYNTAX FAIL RETURN UNKNOWN ENDIF BREAKLOOP LOOP CONTINUELOOP ELSEE ENDIF EOF */
    switch (flsc) {
        case OK :
             return SYNTAX;    /* 可以接受的傳回原因只有 ENDIF 與 ELSEE。 OK 反而是問題，不管它是哪來的。OK can be from these flow control words */
        case ENDIFF :
             return OK;
        case ELSEE :
             return flsc = pass_else();
             /* [x] 不應該有從 ELSEE 回來的可能吧？ 是個錯誤吧？  -hcchen5600 2008/12/20 19:08 */
             /* 不是個錯誤！稱做 skip_else() 也許更清楚一點。從 else 回來以後，要把 else .. endif 之間 skip 掉。 */
        default :
             return flsc; /* OK SYNTAX FAIL RETURN UNKNOWN ENDIF BREAKLOOP LOOP CONTINUELOOP ELSEE ENDIF EOF */
    }

    /* F50 暫時版 直接翻譯F50 之前的舊版成 switch - case 格式而已，方便釐清流程結構
    switch (err) {
        case OK :
             return FAIL;
        case FAIL :
             if (str_compare(token,"ENDIF")==0) return OK;  這實在有夠勉強，應該讓 forth()/NEST 直接傳回對的值，表達它的結束方式
             if (str_compare(token,"ELSE")==0) {
                 return pass_else();
             }
             return FAIL;
        default :
             return err;
    }
    */
    /* F44 之前的舊版，流程不清。可憐當年整天忙亂，如此倉促行事。
        if ((err = forth())==OK) return FAIL;
        if (err!=FAIL) return err;
        if (str_compare(token,"ENDIF")==0) return OK;
        if (str_compare(token,"ELSE")==0) {
            return pass_else();
        }
        return FAIL;
    */
}


/* 先把 if ... else 之間的東西全跳過。 if else endif 可以 nested 這是主要難點，要能正確地全部 skip 過去，還要能發現錯誤。
   這事有點像 pass_if() 但是又不太一樣，只好自己做了。目標是找到那個對的 else 或 endif。掃下去，看到別的字都跳過，看到 if
   請 pass_if() 負責把它整個 skip 掉。看到 else 就要開始做事了。看到 else 之前先看到 endif 則結束。
*/
FLSC Else (void)
{   int temp_status;
    char temp_token[WORDSIZE];  /* f50  -hcchen5600 2008/12/20 18:50 */
    for (;;) {
        if (next_word(fp, temp_token)!=OK) return EOF;   /* 要一直找下去 先把 if ... else 之間的東西全跳過。 */
        if (str_compare(temp_token,"ENDIF") == 0) return OK;   /* 看到 else 之前先看到 endif 則結束。 傳回 OK */
        if (str_compare(temp_token,"IF") == 0) {        /* 看到 if 請 pass_if() 負責把它整個 skip 掉。 */
            if ((temp_status=pass_if())==OK) continue;
            return temp_status;   /* OK SYNTAX FAIL RETURN UNKNOWN ENDIF BREAKLOOP LOOP CONTINUELOOP ELSEE ENDIF EOF */
        }

        if (str_compare(temp_token,"ELSE") == 0) {  /* 看到 else 就要開始做事了。 */

     /*     if((temp_status=forth())==OK) return FAIL;
            if (temp_status!=FAIL) return temp_status;
            if (str_compare(temp_token,"ENDIF")==0) return OK;
            if (str_compare(temp_token,"ELSE")==0) return SYNTAX;
            return temp_status;
     */

            temp_status = forth();    /* for easier debug */
            switch (temp_status) {
                case OK :
                     return SYNTAX;    /* 可以接受的傳回原因只有 ENDIF。 OK 反而是問題，不管它是哪來的。can be from these flow control words */
                case ENDIFF :
                     return OK;
                case ELSEE :
                     return SYNTAX;
                     /* 不應該有從 ELSEE 回來的可能, 這一定是個錯誤！ */
                default :
                     return temp_status; /* OK SYNTAX FAIL RETURN UNKNOWN ENDIF BREAKLOOP LOOP CONTINUELOOP ELSEE ENDIF EOF */
            }


        }
    }
}

/* 把 else ... endif 之間的東西全跳過。 if else endif 可以 nested 這是主要難點，要能正確地全部 skip 過去，還要能發現錯誤。
   原理不難，目標是找到那個對的 endif。掃下去，看到別的字都跳過，看到 if 要小心！請 pass_if() 負責把它整個 skip 掉。看到 else 肯定是個錯誤。
   這個 loop 是不做事的，光負責把不該做的 words 跳過，所以不能叫用 NEST(forth())
*/
FLSC pass_else(void)    /* 可能的傳回值有 OK SYNTAX EOF and see what pass_if() may return ? */
{   int temp_status;
    char temp_token[WORDSIZE];

    for (;;) {
        if (next_word(fp, temp_token)!=OK) return EOF;  /* 指向下一個要審查的 word */
        if (str_compare(temp_token, "IF") == 0) {
            if ((temp_status=pass_if())!=OK) return temp_status;  /* 看到 if 要小心！請 pass_if() 負責把它整個 skip 掉。 */
            continue;                                  /* OK SYNTAX FAIL RETURN UNKNOWN ENDIF BREAKLOOP LOOP CONTINUELOOP ELSEE ENDIF EOF */
        }
        if (str_compare(temp_token,"ELSE") == 0) {   /* 看到 else 肯定是個錯誤。 */
            return SYNTAX;
        }
        if (str_compare(temp_token,"ENDIF") == 0) {   /* 終於找到目標了，成功回返 */
            return OK;
        }
    }
}

/* 把整個 if ... else ... endif 之間的東西全跳過。 if else endif 可以 nested 這是主要難點，要能正確地全部 skip 過去，還要能發現錯誤。
   原理不難，目標是找到那個對的 endif。掃下去，看到別的字都跳過，看到 if 請 pass_if() 負責把它整個 skip 掉。看到 else 要小心！這個
   else 一定是同一層的，成功 skip 完，任務也就結束了，請 pass_else() 負責把它整個 skip 掉，同時結束本任務。 這個 loop 是不做事的，光
   負責把不該做的 words 跳過，所以不能叫用 NEST(forth())
*/
FLSC pass_if(void)
{   int temp_status;
    char temp_token[WORDSIZE];
    for (;;) {
        if (next_word(fp,temp_token)!=OK) return EOF;      /* 指向下一個要審查的 word */
        if (str_compare(temp_token,"IF") == 0) {
            if ((temp_status=pass_if())==OK) continue;     /* 看到 if 要小心！請 pass_if() 負責把它整個 skip 掉。 */
            return temp_status;  /* OK SYNTAX FAIL RETURN UNKNOWN ENDIF BREAKLOOP LOOP CONTINUELOOP ELSEE ENDIF EOF */
        }
        if (str_compare(temp_token,"ELSE") == 0) {
            /* if ((temp_status=pass_else())==OK) continue;  <=== Wrong !! This is a trap, be careful.
            return temp_status;
            */
            return pass_else(); /* 小心！這個 else 一定是同一層的，成功 skip 完，任務也就結束了，請 pass_else()
                                   負責把它整個 skip 掉，同時結束本任務。 */
                                /* OK SYNTAX FAIL RETURN UNKNOWN ENDIF BREAKLOOP LOOP CONTINUELOOP ELSEE ENDIF EOF */
        }
        if (str_compare(temp_token,"ENDIF") == 0) {       /* 終於找到目標了，成功回返 */
            return OK;
        }
    }
}

FLSC Do(void)
{   int flsc;
    unsigned begin_position, nest_level;
    char temp_token[WORDSIZE];

    begin_position = sbuf.p;
    nest_level = 1;
    for (;;) {
        /* forth() run from sbuf.p until certain FLSC (see forth.h F Language Syntax Cases ) happened, it returns FLSC */
        if ((flsc=forth())==OK) return SYNTAX;  /* syntax error, because RETURN unexpected.  -hcchen5600 2008/12/25 09:29 */
        switch (flsc) {
                case BREAKLOOP :
                        for (;;) {
                                   /* BREAK or ?BREAK, skip everything before the ending LOOP */
                             if (next_word(fp,temp_token)!=OK)
                                  return EOF;   /* if can't find the ending LOOP then it's like an EOF error */
                             if (str_compare(temp_token,"DO")==0) {     /* 故意放 LOOP DO 字樣在 comment and string 裡，看有沒有問題 */
                                  nest_level += 1;
                             }
                             if (str_compare(temp_token,"LOOP")==0||str_compare(temp_token,"+LOOP")==0||str_compare(temp_token,"-LOOP")==0) {
                                  if (--nest_level) {
                                        continue;
                                  } else {
                                        break;
                                  }
                             }
                        }
                        /* end_position = sbuf.p; */
                        return OK;
                case CONTINUELOOP :
                case MINUSLOOP :
                        sbuf.p = begin_position;
                        auxpush(auxpop()-1);  /* F51 */
                        continue;
                case PLUSLOOP :
                        sbuf.p = begin_position;
                        auxpush(auxpop()+1);  /* F51 */
                        continue;
                case LOOP :
                        sbuf.p = begin_position;
                        continue;
        }
        if (flsc!=FAIL) return flsc; /* something wrong*/
        return FAIL; /* Unknown */
    }
}

void (far * farcall)(unsigned addr_of_tos);       /* farcall is a function pointer        */
void (* nearcall)(unsigned addr_of_tos);          /* nearcall is a function pointer too   */
int (* brk_handler)(void);

void brk_abort(void)
{
    /* ch_deadloop(0x1111); was for debugging PMode ISR when studying Alexie's TUT05 */
    exit(1);
}
int brk_resume(void)
{
    return 1;
}

FLSC ch_forth(char *token)
{
    FILE *fo;
    int i,j,k,l;

    if (str_compare(token,"EXIT")==0||str_compare(token,"BYE")==0) {
        i = stack.tos ? pop() : 0;
        exit(i);
    }
    if (str_compare(token,"?EXIT")==0||str_compare(token,"?BYE")==0) {
        if(pop()) {
            i = stack.tos ? pop() : 0;
            exit(i);
        }
        return OK;
    }
    if (str_compare(token,"DUP")==0) {
        i = pop();
        push(i);
        push(i);
        return OK;
    }
    if (str_compare(token,"2DUP")==0) {
        i = pop(); j = pop();
        push(j); push(i);
        push(j); push(i);
        return OK;
    }
    if (str_compare(token,"DROP")==0) {
        pop();
        return OK;
    }
    if (str_compare(token,"DROPALL")==0) {
        stack.tos = 0;
        return OK;
    }
    if (str_compare(token,"SWAP")==0) {
        i = pop();
        j = pop();
        push(i);
        push(j);
        return OK;
    }
    if (str_compare(token,"2SWAP")==0) {
        i = pop(); j = pop();
        k = pop(); l = pop();
        push(j); push(i);
        push(l); push(k);
        return OK;
    }
    if (str_compare(token,"OVER")==0) {
        i = pop();
        j = pop();
        push(j);
        push(i);
        push(j);
        return OK;
    }
    if (str_compare(token,"2OVER")==0) {
        i = pop(); j = pop();
        k = pop(); l = pop();
        push(l); push(k);
        push(j); push(i);
        push(l); push(k);
        return OK;
    }
    if (str_compare(token,"OVER2")==0) {
        i = pop();
        j = pop();
        k = pop();
        push(k);
        push(j);
        push(i);
        push(k);
        return OK;
    }
    if (str_compare(token,"ROT")==0) {
        i = pop();
        j = pop();
        k = pop();
        push(j);
        push(i);
        push(k);
        return OK;
    }
    if (str_compare(token,"-ROT")==0) {
        i = pop();
        j = pop();
        k = pop();
        push(i);
        push(k);
        push(j);
        return OK;
    }
    /* F51 aux stack commands A> >A A@ */
    if (str_compare(token,"A>")==0) {
        i = auxpop();
        push(i);
        return OK;
    }
    if (str_compare(token,">A")==0) {
        i = pop();
        auxpush(i);
        return OK;
    }
    if (str_compare(token,"A@")==0) {
        auxpush(i=auxpop());
        push(i);
        return OK;
    }
    if (str_compare(token,"PICK")==0) {
        k = pop();
        if (k>=stack.tos) {
            printf("\nPick under stack! %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);  /* F54 */
            stack.error = 1;  /* f54 */
        } else {
            j = stack.tos - k;
            l = stack.data[j];
            for (i=j; i<stack.tos; i++) {
                stack.data[i] = stack.data[i+1];
            }
            stack.data[i] = l;
        }
        return OK;
    }
    if (str_compare(token,"ROLL")==0) {
        k = pop();
        if (k>=stack.tos) {
            printf("\nRoll under stack! %04x:%04x\n", _DS, &sbuf.b[sbuf.p]);  /* F54 */
            stack.error = 1;  /* f54 */
        } else {
            j = stack.tos - k;
            l = stack.data[stack.tos];
            for (i=stack.tos; i>j; i--) {
                stack.data[i] = stack.data[i-1];
            }
            stack.data[i] = l;
        }
        return OK;
    }
    if ((str_compare(token,"STACK")==0)||(str_compare(token,".S")==0)) {
        printf("\n TOS          Data           Aux\n");
        for (i=max(stack.tos,auxstack.tos); i>=0; i--) {
            if (i==0) {
                printf("%4d     %04x:%04x", i, _DS, stack.data);
            } else if (i <= stack.tos) {
                printf("%4d %6d [%4Xh] ", i, stack.data[i], stack.data[i]);
            } else {
                printf("%4d        [     ] ", i);
            }
            if (i==0) {
                printf("      %04x:%04x", _DS, auxstack.data);
            } else if (i <= auxstack.tos) {
                printf("%6d [%4Xh]\n", auxstack.data[i], auxstack.data[i]);
            } else {
                printf("       [     ]\n");
            }
        }
        putchar('\n');
        return OK;
    }
    if (str_compare(token,"?RET")==0) {
        if (pop()) return RETURN;
        else return OK;
    }
    if (str_compare(token,"LOOP")==0) {
        return LOOP;                  /* do ... loop */
    }
    if (str_compare(token,"+LOOP")==0) {
        return PLUSLOOP;                  /* do ... loop */
    }
    if (str_compare(token,"-LOOP")==0) {
        return MINUSLOOP;                  /* do ... loop */
    }
    if (str_compare(token,"CONTINUE")==0) {
        return CONTINUELOOP;          /* do ... loop */
    }
    if (str_compare(token,"BREAK")==0) {
        return BREAKLOOP;             /* do ... break ... loop */
    }
    if (str_compare(token,"?BREAK")==0) {
        if (pop()) return BREAKLOOP;  /* do ... ?break ... loop */
        else return OK;
    }
    if ((str_compare(token,";")==0) || (str_compare(token,"RET")==0)) {
        return RETURN;
    }
    if (str_compare(token,"IF")==0) {
        if (!inScriptMode(token)) return SYNTAX;  /* is not in script mode */
        if (pop()) {
            return If();
        } else {
            return Else();
        }
    }
    if (str_compare(token,"ELSE")==0) {
        return ELSEE;
    }
    if (str_compare(token,"ENDIF")==0) {
        return ENDIFF;
    }
    if (str_compare(token,"DO")==0) {
        if (!inScriptMode(token)) return SYNTAX;  /* is not in script mode */
        return Do();
    }
    if (str_compare(token,"(D")==0) {
        if (!debug) {
            while (next_word(fp,token)!=EOF) {
                if (str_compare(token,"D)")==0) break;
            }
        }
        return OK; /* EOF can be found later */
    }
    if (str_compare(token,"D)")==0) {
        return OK;
    }
    if (str_compare(token,"LOADFILE")==0||str_compare(token,"LOADFO")==0) {
        /* [buffer limit_length -- err] */
        next_word(fp,ss);
        fo = fopen(ss, "rb");
        i = pop();  /* limit length */
        j = pop();  /* buffer */
        if ((fo==NULL) || (read(fileno(fo), (char *) j, i)==-1)) {
          push(-1); /* failed */
        } else {
          push(0);
        }
        return OK;
    }
    if (str_compare(token,"FARCALL")==0) {
        asm Global label_farcall      /* get the address in map file for debug */
        asm label_farcall:
        /* [ ... seg off -- ... ] */
        (unsigned long) farcall = (unsigned) pop();
        (unsigned long) farcall+= (unsigned long) pop() << 16;
        farcall((int) stack.data + 2*stack.tos);
        return OK;
    }
    if (str_compare(token,"NEARCALL")==0) {      /* When in protected mode, nearcall is necessary to avoid changing CS  -hcchen5600 2008/12/11 09:47 */
        asm Global label_nearcall      /* get the address in map file for debug */
        asm label_nearcall:
        /* [ ... offset -- ... ] */
        (unsigned) nearcall = (unsigned) pop();

        /* nearcall((int) stack.data + 2*stack.tos);  */
        asm mov     ax,word ptr DGROUP:_stack
        asm shl     ax,1
        asm mov     dx,offset DGROUP:_stack+2
        asm add     ax,dx
        asm push    ax
        asm call    word ptr CS:_nearcall
        asm pop     cx

        return OK;
    }
    if (str_compare(token,"CALL")==0) {   /* 'call' is actually spawn, shell out to DOS to run a program then return with errorlevel at TOS  -hcchen5600 2008/12/11 09:46 */
        if (fp==stdin) fgets(ss,199,fp);          /* to call subroutine, use 'nearcall' or 'farcall' command  -hcchen5600 2008/12/11 09:46 */
        else bgets(ss,199);
        for (i=0;i<200;i++) if (ss[i]==' '||ss[i]=='\t'||ss[i]=='\n'||ss[i]=='\r') continue; else break; /* advance to next non white space char */
        for (j=0;i<200;j++) {
            *((char **) &ss[200+j*2]) = &ss[i];
            for (;i<200;i++) {
                if (ss[i]==' '||ss[i]=='\t') {
                    ss[i++] = '\0';
                    break;
                }
                if (ss[i]=='\n'||ss[i]=='\r') {
                    ss[i] = '\0';
                    i = 200;  /* to break the outer loop too */
                    break;
                }
            }
            for (;i<200;i++) if (ss[i]==' '||ss[i]=='\t'||ss[i]=='\n'||ss[i]=='\r') continue; else break; /* advance to next non white space char */
        }
        *((char **) &ss[200+j*2]) = NULL;
        push(spawnvp(P_WAIT, *((char**) &ss[200]), &ss[200]));
        return OK;
    }
    if (str_compare(token,"BREAKON")==0) {
        brk_handler = (int (*)(void)) brk_abort;
        ctrlbrk(brk_handler);
        return OK;
    }
    if (str_compare(token,"BREAKOFF")==0) {
        brk_handler = brk_resume;
        ctrlbrk(brk_handler);
        return OK;
    }

    if (str_compare(token,"MAP")==0) {  /* [ n -- entry ]  get C function entry offset */
        push((unsigned) label_list[pop()]);
        return OK;
    }
    return FAIL;
}


