/*
**  dicenv.c
**  
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "undocdos.h"           /* defines structures   */
#include "forth.h"

#ifndef FP_SEG
#define FP_SEG(f) (*((unsigned *)&(f) + 1))
#endif

#ifndef FP_OFF
#define FP_OFF(f) (*((unsigned *)&(f)))
#endif

WORD parent, self;

void savenv(char far *lp)
{   int j,k,p;

    for (p=j=0; environ[j][0]; j++) {
        for (k=0; environ[j][k]; k++) {
            lp[p++] = environ[j][k];
        }
        lp[p++] = '\0';
    }
    lp[p++] = '\0';
}

FLSC putenviron(char *ss)   /* 此 local ss 非彼 glogal ss */
{          
    char *newenv;
    int  ll;

    ll = strlen(ss) - 1;
    if ((ss[ll]=='.') && (ss[ll-1]=='=')) {
        sprintf(&ss[ll],"%d",pop());
    } else if ((ss[ll]=='$') && (ss[ll-1]=='.')) {
        sprintf(&ss[ll-1],"%X",pop());
    }
    if ((newenv=malloc(strlen(ss)))==NULL) {
        puts("malloc failed!");
        return FAIL;
    }
    strcpy(newenv,ss);
    if (putenv(newenv)==OK) {
        return OK;
    } else {
        puts("putenv failed!");
        return FAIL;
    }
}

int jj;
FLSC dic_env(char *token)
{
    int i;
    char *p;

    if (token[0]=='%' && token[1]=='$') {   /* 用 getenv 取得 env 變數的 string 位址，然後用 sscanf 而無所不為。這個命令非必要。 -hcchen5600 2008/12/20 15:23 */
        jj = 0;
        p = getenv(&token[2]);
        if (p) sscanf(p, "%X", &jj);
        push(jj);
        return OK;
    }
    if (token[0]=='%') {
        push(atoi(getenv(&token[1])));
        return OK;
    }
    if (str_compare(token,"GETENV")==0) {  /* 取得環境變數的位址 */
        next_word(fp, token);      
        push((int)getenv(token));
        return OK;
    }
    if (str_compare(token,"PUTENV")==0) {
        next_word(fp,token);
        putenviron(token);
        return OK;
    }
    if (str_compare(token,"PREVPSP")==0) {
        push(PARENT(pop()));
        return OK;
    }
    if (str_compare(token,"THISPSP")==0) {
        push(_psp);
        return OK;
    }
    if (str_compare(token,"PSP2ENV")==0) {
        push(ENV_FM_PSP(pop()));
        return OK;
    }
    if (str_compare(token,"SAVEENV")==0) {
        savenv((char far *)MK_FP(pop(),0));
        return OK;
    }
    if (str_compare(token,".ENV")==0) {
        for (i=0; environ[i][0]; i++) {
            puts(environ[i]);
        }
        return OK;
    }
    return FAIL; /* Unknown input */
}
