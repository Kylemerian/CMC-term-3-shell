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
    QUOTEFLAG,
    AMPERFLAG
};

enum inout {
    IN,
    OUT,
    OUTAPP
};

void init(list ** head)
{
    *head = malloc(sizeof(** head));
    (*head) -> str = NULL;
    (*head) -> next = NULL;
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
        if (head -> next != NULL)
            printf("%s\n", head -> str);
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
    tmp -> str = malloc(lenbuff + 1);
    strncpy(tmp -> str, str, lenbuff);
    tmp -> str[lenbuff] = 0;
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
    /*printf("%s ", tpointer->str);*/
    return tpointer->str;
}

int changeIO(list * head, int * inOut)
{
    int fd;
    int errflag = 0;
    if(inOut[IN]){
        fd = open(getName(inOut[IN], head), O_RDWR | O_CREAT);
        if(fd != -1)
            dup2(fd, 0);
        else
            errflag = 1;
    }
    if(inOut[OUTAPP]){
        fd = open(getName(inOut[OUTAPP], head),O_RDWR|O_APPEND|O_CREAT,0777);
        if(fd != -1)
            dup2(fd, 1);
        else
            errflag = 1;
    }
    if(inOut[OUTAPP] == 0 && inOut[OUT]){
        fd = open(getName(inOut[OUT], head), O_RDWR | O_CREAT, 0777);
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
    return (iterator - 1);
}

void handleIOquote(int lastc, int c, int * inOut, list * headlist, int i)
{
    int size = listsize(headlist) + 1;
    if(i > 0)
        size++;
    if(c == '<')
        if(inOut[IN] == 0)
            inOut[IN] = size;
        else
            inOut[IN] = -1;
    else if(c == '>' && lastc == '>')
        if(inOut[OUTAPP] == 0)
            inOut[OUTAPP] = size;
        else
            inOut[IN] = -1;
    else if(c == '>')
        if(inOut[OUT] == 0)
            inOut[OUT] = size;
        else
            inOut[IN] = -1;
    else
        inOut[IN] = -1;
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
    return (c == ' ') || (c == '\t') || (c == '&') || (c == '>') || (c == '<');
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

char ** makearr(list * headlist, int size, int * inOut)
{
    list * tmp = headlist;
    int i, j;
    int sizeIO = countSizeIO(inOut);
    j = size - 1 - sizeIO;
    char ** arr = malloc((size + 1) * sizeof(*arr));
    for(i = size - 1; i >= 0; i--, tmp = tmp->next){
        if(!inIO(i + 1, inOut)){
            arr[i] = malloc(strlen(tmp->str) + 1);
            strncpy(arr[i], tmp->str, strlen(tmp->str));
            arr[i][strlen(tmp->str)] = 0;
            j--;
        }
    }
    arr[size - sizeIO] = (char*)NULL;
    return arr;
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

void execute(list * headlist, int mode, int * inOut)
{
    int j, pid;
    int sizeIO = countSizeIO(inOut);
    int size = listsize(headlist);
    char ** arr = makearr(headlist, size, inOut);
    if(!strcmp(arr[0], "cd"))
        changedir(arr, size);
    else{
        pid = fork();
        if(pid == 0){
            if(!changeIO(headlist, inOut)){
                execvp(arr[0], arr);
            }
            perror(NULL);
            exit(1);
        }
        if(mode == 0)
            while(wait(NULL) != pid)
                ;
    }
    for(j = 0; j < size - sizeIO; j++)
          free(arr[j]);
    free(arr);
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
    if(inOut[IN] && (inOut[IN] == inOut[OUT] || inOut[IN] == inOut[OUTAPP]))
        return 1;
    for(i = 0; i < 3; i++)
        if(inOut[i] > size)
            return 1;
    if(inOut[0] == -1)
        return 1;
    return 0;
}

void iscorrectcmd(int * flags, int chr, list ** head, int * inOut)
{
    int i;
    int mode = (flags[AMPERFLAG] == 1);
    if (flags[QUOTEFLAG] || (flags[AMPERFLAG] > 1) ||
        (flags[AMPERFLAG] == 1 && chr != '&') || isWrongIO(*head, inOut))

        printf("incorrect input\n");
    else if((*head)->str != NULL){
        execute(*head, mode, inOut);
        /*printlist(*head);*/
    }
    flags[QUOTEFLAG] = 0;
    flags[AMPERFLAG] = 0;
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
            if ((!divide(c) && c != '\"') || (divide(c) && flags[QUOTEFLAG])){
                if (i >= lenbuff - 1)
                    buff = extendbuff(buff, &lenbuff);
                buff[i++] = c;
            }
            else if (isIOsymbol(c))
                handleIOquote(lastchar, c, inOutPos, headlist, i);
            else if (c == '\"')
                flags[QUOTEFLAG] = !flags[QUOTEFLAG];
            else if (c == '&')
                flags[AMPERFLAG]++;
            if (i != 0 && !flags[QUOTEFLAG] && spaceTabIOquote(c)) {
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
