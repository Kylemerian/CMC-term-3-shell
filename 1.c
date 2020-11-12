#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

typedef struct list {
    char * str;
    struct list * next;
}
list;

enum flag {
    quoteflag,
    amperflag
};

enum inout {
    in,
    out,
    outapp
};

void init(list ** head)
{
    *head = NULL;
    printf(">> ");
}

char * extendbuff(char * buff, int * lenbuff)
{
    char * newBuff = malloc((*lenbuff) * 2);
    strncpy(newBuff, buff, (*lenbuff) - 1);
    *lenbuff *= 2;
    free(buff);
    return newBuff;
}

void printlist(list * head)
{
    if (head != NULL) {
        printlist(head -> next);
        if (head -> str != NULL)
            printf("%s\n", head -> str);
        else
            printf("NULL\n");
    }
}

void printRecurs(list * headlist)
{
    printlist(headlist);
}

void freemem(list * headlist)
{
    if (headlist != NULL) {
        if (headlist -> str != NULL)
            free(headlist -> str);
        list * tmp = headlist;
        headlist = headlist -> next;
        free(tmp);
        freemem(headlist);
    }
}

list * addtolist(list * head, char * str, int lenbuff)
{
    list * tmp = malloc(sizeof(* tmp));
    if(lenbuff != -1){
        tmp -> str = malloc(lenbuff + 1);
        strncpy(tmp -> str, str, lenbuff);
        tmp -> str[lenbuff] = 0;
    }
    else
        tmp -> str = NULL;
    tmp -> next = head;
    return tmp;
}

char * getName(int key, list * headlist)
{
    int it = 1;
    list * tpointer = headlist;
    while(tpointer != NULL){
        if(key == it)
            return tpointer->str;
        it++;
    }
    return tpointer->str;
}

int changeIO(list * head, int * inOut)
{
    int fd;
    int errflag = 0;
    if(inOut[in]){
        fd = open(getName(inOut[in], head), O_RDWR | O_CREAT);
        if(fd != -1)
            dup2(fd, 0);
        else
            errflag = 1;
    }
    if(inOut[outapp]){
        fd = open(getName(inOut[outapp], head),O_RDWR|O_APPEND|O_CREAT,0777);
        if(fd != -1)
            dup2(fd, 1);
        else
            errflag = 1;
    }
    if(inOut[outapp] == 0 && inOut[out]){
        fd = open(getName(inOut[out], head), O_RDWR | O_CREAT, 0777);
        if(fd != -1)
            dup2(fd, 1);
        else
            errflag = 1;
    }
    return errflag;
}

int listsize(list * headlist)
{
    int iterator = 0;
    list * tmp = headlist;
    while(tmp != NULL){
        iterator++;
        tmp = tmp->next;
    }
    return iterator;
}

void handleIOquote(int lastc, int c, int * inOut, list * headlist, int i)
{
    int size = listsize(headlist) + 1;
    if(i > 0)
        size++;
    if(c == '<')
        if(inOut[in] == 0)
            inOut[in] = size;
        else
            inOut[in] = -1;
    else if(c == '>' && lastc == '>')
        if(inOut[outapp] == 0)
            inOut[outapp] = size;
        else
            inOut[in] = -1;
    else if(c == '>')
        if(inOut[out] == 0)
            inOut[out] = size;
        else
            inOut[in] = -1;
    else
        inOut[in] = -1;
}

int isIOsymbol(int c)
{
    return ((c == '>') || (c == '<'));
}

int spaceTab(int c)
{
    return ((c == '\t') || (c == ' '));
}

int spaceTabIOquote(int c)
{
    return ((c == ' ') || (c == '\t') || (c == '<') || (c == '>'));
}

int divide(int c)
{
    return (c == ' ') || (c == '\t') || (c == '&') || (c == '>') || (c == '<') || (c == '|');
}

int inIO(int key, int * inOut)
{
    int i;
    int inFlag = 0;
    for(i = 0; i < 3; i++)
        if(inOut[i] == key){
            inFlag = 1;
            break;
        }
    return inFlag;
}

int countSizeIO(int * inOut)
{
    int res = 0, i = 0;
    for (i = 0; i < 3; i++)
        if(inOut[i] != 0)
            res++;
    if(inOut[2] > 0)
        res--;
    return res;
}

int getNumPrcs(char ** arr, int sizeArr)
{
    int cnt = 0;
    int i;
    for(i = 0; i < sizeArr; i++)
        if(arr[i] == NULL)
            cnt++;
    return cnt;
}

int getNextCmd(char ** arr, int prevInd)
{
    int i;
    for(i = prevInd; arr[i] != NULL; i++)
        ;
    return (i + 1);
}

char ** makearr(list * headlist, int size, int * inOut)
{
    list * tmp = headlist;
    int i, j;
    int sizeIO = countSizeIO(inOut);
    j = size - 1 - sizeIO;
    char ** arr = malloc((size - sizeIO + 1) * sizeof(*arr));
    for(i = size - 1; i >= 0; i--, tmp = tmp->next){
        if(!inIO(i + 1, inOut)){
            if(tmp -> str != NULL){
                arr[i] = malloc(strlen(tmp->str) + 1);
                strncpy(arr[i], tmp->str, strlen(tmp->str));
                arr[i][strlen(tmp->str)] = 0;
            }
            else
                arr[i] = NULL;
            j--;
        }
    }
    arr[size - sizeIO] = NULL;
    return arr;
}

int sizeForCD(char ** arr)
{
    int i = 0;
    while(arr[i] != NULL)
        i++;
    return i;
}

void changedir(char ** arr, int size)
{
    int status;
    if(size == 2){
        status = chdir(arr[1]);
        if(status == -1)
            perror(arr[1]);
    }
    else if(size == 1){
        printf("Too few args\n");
    }
    else
        printf("Too many args\n");
}

void freeArrs(char ** arr, int ** fd, int size, int numPrcs)
{
    int k;
    for(k = 0; k < numPrcs; k++)
        free(fd[k]);
    for(k = 0; k < size; k++){
        if(arr[k] != NULL)
            free(arr[k]);
    }
    free(arr);
    free(fd);
}

int ** makefds(int numPrcs)
{
    int k;
    int ** fd = malloc(sizeof(*fd) * numPrcs);
    for(k = 0; k < numPrcs; k++)
        fd[k] = malloc(2 * sizeof(int));
    return fd;
}

void execute(list * headlist, int mode, int * inOut)
{
    int j, k, pid, nextCmd = 0;
    int sizeIO = countSizeIO(inOut);
    int size = listsize(headlist);
    char ** arr = makearr(headlist, size, inOut);
    int numPrcs = getNumPrcs(arr, size - sizeIO + 1);
    int ** fd = makefds(numPrcs);
    for(k = 0; k < numPrcs; k++){
        pipe(fd[k]);
        if(!strcmp(arr[nextCmd], "cd"))
            changedir(&arr[nextCmd], sizeForCD(&arr[nextCmd]));
        else{
            pid = fork();
            if(pid == 0){
                /*if(!changeIO(headlist, inOut)){
                    execvp(arr[nextCmd], &arr[nextCmd]);
                }*/
                if(k != 0)
                    dup2(fd[k - 1][0], 0);
                if(k != numPrcs - 1)
                    dup2(fd[k][1], 1);
                close(fd[k][0]);
                execvp(arr[nextCmd], &arr[nextCmd]);
                perror(NULL);
                exit(1);
            }
            close(fd[k][1]);
            if(k != 0)
                close(fd[k - 1][0]);
            if(mode == 0)
                while(wait(NULL) != pid)
                    ;
        }
        nextCmd = getNextCmd(arr, nextCmd);
    }
    freeArrs(arr, fd, size - sizeIO, numPrcs);
}

void reinit(list ** headlist)
{
    freemem(*headlist);
    init(headlist);
}

void processinglast(int * iterator, char * buff, list ** headlist)
{
    if(*iterator != 0)
        *headlist = addtolist(*headlist, buff, *iterator);
    *iterator = 0;
}

int isWrongIO(list * headlist, int * inOut)
{
    int size = listsize(headlist);
    int i = 0;
    if(inOut[in] && (inOut[in] == inOut[out] || inOut[in] == inOut[outapp]))
        return 1;
    for(i = 0; i < 3; i++)
        if(inOut[i] > size)
            return 1;
    if(inOut[0] == -1)
        return 1;
    return 0;
}

int isWrongPipe(list * head)
{
    int errflag = 0;
    int pared = 0;
    list * tmp = head;
    if(tmp != NULL && tmp -> str == NULL)
        errflag = 1;
    while(tmp != NULL){
        if(tmp -> str == NULL && pared == 1)
            errflag = 1;
        if(tmp -> str == NULL)
            pared = 1;
        else 
            pared = 0;
        tmp = tmp -> next;
    }
    tmp = head;
    if(tmp != NULL){
        while(tmp -> next != NULL)
            tmp = tmp -> next;
        if(tmp -> str == NULL)
            errflag = 1;
    }
    return errflag;
}

void iscorrectcmd(int * flags, int chr, list ** head, int * inOut)
{
    int i;
    int mode = (flags[amperflag] == 1);
    if (flags[quoteflag] || (flags[amperflag] > 1) 
        || (flags[amperflag] == 1 && chr != '&') 
        || isWrongIO(*head, inOut) || isWrongPipe(*head))

        printf("incorrect input\n");
    else if((*head) != NULL){
        execute(*head, mode, inOut);
        /*printlist(*head);*/
    }
    flags[quoteflag] = 0;
    flags[amperflag] = 0;
    for(i = 0; i < 3; i++)
        inOut[i] = 0;
}

void killbg()
{
    while(wait4(-1, NULL, WNOHANG, NULL) > 0)
        ;
}

void freememory(list * headlist, char * buff)
{
    freemem(headlist);
    free(buff);
    printf("\n");
}

int main()
{
    int c, lastchar = 0, lenbuff = 8, i = 0;
    int flags[2] = {0, 0};
    int inOutPos[3] = {0, 0, 0};
    char * buff = malloc(lenbuff);
    list * headlist = NULL;
    init(&headlist);
    while ((c = getchar()) != EOF) {
        if (c != '\n') {
            if ((!divide(c) && c != '\"') || (divide(c) && flags[quoteflag])){
                if (i >= lenbuff - 1)
                    buff = extendbuff(buff, &lenbuff);
                buff[i++] = c;
            }
            else if (isIOsymbol(c))
                handleIOquote(lastchar, c, inOutPos, headlist, i);
            else if (c == '\"')
                flags[quoteflag] = !flags[quoteflag];
            else if (c == '&')
                flags[amperflag]++;
            else if (c == '|'){
                if(i != 0)
                    headlist = addtolist(headlist, buff, i);
                headlist = addtolist(headlist, buff, -1);
                i = 0;
            }
            if (i != 0 && !flags[quoteflag] && spaceTabIOquote(c)) {
                headlist = addtolist(headlist, buff, i);
                i = 0;
            }
            if(!spaceTab(c))
                lastchar = c;
        }
        else{
            processinglast(&i, buff, &headlist);
            iscorrectcmd(flags, lastchar, &headlist, inOutPos);
            reinit(&headlist);
            killbg();
        }
    }
    freememory(headlist, buff);
    return 0;
}
